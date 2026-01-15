#include "pch.h"
#include "AFilePackage.h"
#include "AFPI.h"
#include "AStringConv.h"
#include "zlib.h"

namespace
{
    std::unique_ptr<AFilePackage> g_globalPackage;

    bool iequals(std::string_view a, std::string_view b)
    {
        return a.size() == b.size() &&
            std::equal(a.begin(), a.end(), b.begin(),
                [](char ca, char cb) {
                    return std::tolower(static_cast<unsigned char>(ca)) ==
                        std::tolower(static_cast<unsigned char>(cb));
                });
    }

    // Helper: case-insensitive string compare
    std::string NormalizeFileName(std::string_view input)
    {
        std::string result(input);

        // Fix path separators
        std::replace(result.begin(), result.end(), '/', '\\');

        // Leading .\ Removal
        if (result.size() >= 2 && result[0] == '.' && result[1] == '\\')
            result.erase(0, 2);

        // Trim trailing spaces
        result.erase(std::find_if(result.rbegin(), result.rend(),
            [](unsigned char ch) { return ch != ' '; }).base(),
            result.end());

        // Trim leading spaces (optional)
        result.erase(result.begin(),
            std::find_if(result.begin(), result.end(),
                [](unsigned char ch) { return ch != ' '; }));

        return result;
    }
}

AFilePackage::~AFilePackage()
{
    AFilePackage::Close();
}

bool AFilePackage::Open(std::wstring_view pckPath, AFPCK_OPENMODE mode)
{
    if (m_packageFile.is_open())
        Close();

    m_mode = mode;
    m_hasChanged = false;
    m_readOnly = false;

    std::ios::openmode fmode;
    if (mode == AFPCK_OPENMODE::AFPCK_CREATENEW)
    {
        fmode = std::ios::binary | std::ios::out | std::ios::trunc;
        m_packageFile.open(pckPath, fmode);
        if (!m_packageFile)
        {
            AFERRLOG(L"AFilePackage::Open(), Can not create file [{}]", pckPath);
            return false;
        }

        // Initialize header
        m_header = AFPCK_FILEHEADER{};
        m_header.dwVersion = CURRENT_VERSION;

        constexpr std::string_view desc = "Angelica File Package, Beijing E-Pie Entertainment Corporation 2002~2008. All Rights Reserved. ";
        const size_t maxDescLen = sizeof(m_header.szDescription) - 1;
        const size_t copyDescLen = std::min(desc.size(), maxDescLen);
        std::copy(desc.begin(), desc.begin() + copyDescLen, m_header.szDescription);
        m_header.szDescription[copyDescLen] = '\0';

        m_fileEntries.clear();
        m_readOnly = false;
    }
    else
    {
        // Try read-write first, then read-only
        fmode = std::ios::binary | std::ios::in | std::ios::out;
        m_packageFile.open(pckPath, fmode);
        if (!m_packageFile)
        {
            fmode = std::ios::binary | std::ios::in;
            m_packageFile.open(pckPath, fmode);
            if (!m_packageFile)
            {
                AFERRLOG(L"AFilePackage::Open(), Can not open file [{}]", pckPath);
                return false;
            }

            m_readOnly = true;
        }
        else
            m_readOnly = false;

        // Read version from end
        m_packageFile.seekg(-static_cast<std::streamoff>(sizeof(std::uint32_t)), std::ios::end);
        std::uint32_t version = 0;
        m_packageFile.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (version != CURRENT_VERSION)
        {
            AFERRLOG(L"AFilePackage::Open(), Incorrect version! Got {:#x}", version);
            return false;
        }

        if (!LoadEntries())
            return false;

        ResortEntries();
    }

    // Prepare compression buffer if needed
    if (IsAFCompressionEnabled())
        m_compressionBuffer.resize(1024 * 1024);

    return true;
}

bool AFilePackage::Close()
{
    if (!m_packageFile.is_open())
        return true;

    if (m_mode == AFPCK_OPENMODE::AFPCK_OPENEXIST && m_hasChanged)
        SaveEntries();
    else if (m_mode == AFPCK_OPENMODE::AFPCK_CREATENEW)
        SaveEntries();

    m_packageFile.close();
    m_fileEntries.clear();
    m_compressionBuffer.clear();
    m_hasChanged = false;

    return true;
}

bool AFilePackage::AppendFile(std::wstring_view fileName, std::span<const std::byte> fileData)
{
    if (m_readOnly)
    {
        AFERRLOG(L"AFilePackage::AppendFile(), Read-only package");
        return false;
    }

    AFPCK_FILEENTRY newEntry{};
    std::string normalized = NormalizeFileName(fileName);
    const size_t maxNameLen = sizeof(newEntry.szFileName) - 1;
    const size_t copyNameLen = std::min(normalized.size(), maxNameLen);
    std::copy(normalized.begin(), normalized.begin() + copyNameLen, newEntry.szFileName);
    newEntry.szFileName[copyNameLen] = '\0';

    std::uint32_t compressedLen = static_cast<std::uint32_t>(fileData.size());
    if (IsAFCompressionEnabled())
    {
        if (m_compressionBuffer.size() < fileData.size())
            m_compressionBuffer.resize(fileData.size());

        uLongf destLen = static_cast<uLongf>(m_compressionBuffer.size());
        int result = compress2(
            reinterpret_cast<Bytef*>(m_compressionBuffer.data()),
            &destLen,
            reinterpret_cast<const Bytef*>(fileData.data()),
            static_cast<uLong>(fileData.size()),
            Z_BEST_SPEED
        );

        if (result == Z_OK && destLen < fileData.size())
            compressedLen = static_cast<std::uint32_t>(destLen);
    }

    newEntry.dwOffset = m_header.dwEntryOffset;
    newEntry.dwLength = static_cast<std::uint32_t>(fileData.size());
    newEntry.dwCompressedLength = compressedLen;

    // Write data
    m_packageFile.seekp(m_header.dwEntryOffset);
    if (compressedLen < newEntry.dwLength)
        m_packageFile.write(reinterpret_cast<const char*>(m_compressionBuffer.data()), compressedLen);
    else
        m_packageFile.write(reinterpret_cast<const char*>(fileData.data()), fileData.size());

    m_header.dwEntryOffset += compressedLen;

    m_fileEntries.push_back(newEntry);
    m_hasChanged = true;
    m_hasSorted = false;

    return true;
}

bool AFilePackage::RemoveFile(std::wstring_view fileName)
{
    if (m_readOnly)
    {
        AFERRLOG(L"AFilePackage::RemoveFile(), Read-only package");
        return false;
    }

    int index = -1;
    AFPCK_FILEENTRY entry;
    if (!GetFileEntry(fileName, entry, &index))
    {
        AFERRLOG(L"AFilePackage::RemoveFile(), File not found: {}", fileName);
        return false;
    }

    m_fileEntries.erase(m_fileEntries.begin() + index);
    m_hasChanged = true;
    m_hasSorted = false;

    return true;
}

#ifdef ReplaceFile
#pragma push_macro("ReplaceFile")
#undef ReplaceFile
bool AFilePackage::ReplaceFile(std::wstring_view fileName, std::span<const std::byte> fileData)
#endif
{
    if (m_readOnly)
    {
        AFERRLOG(L"AFilePackage::ReplaceFile(), Read-only package");
        return false;
    }

    int index = -1;
    AFPCK_FILEENTRY entry;
    if (!GetFileEntry(fileName, entry, &index))
    {
        AFERRLOG(L"AFilePackage::ReplaceFile(), File not found: {}", fileName);
        return false;
    }

    std::uint32_t compressedLen = static_cast<std::uint32_t>(fileData.size());
    if (IsAFCompressionEnabled())
    {
        if (m_compressionBuffer.size() < fileData.size())
            m_compressionBuffer.resize(fileData.size());

        uLongf destLen = static_cast<uLongf>(m_compressionBuffer.size());
        int result = compress2(
            reinterpret_cast<Bytef*>(m_compressionBuffer.data()),
            &destLen,
            reinterpret_cast<const Bytef*>(fileData.data()),
            static_cast<uLong>(fileData.size()),
            Z_BEST_SPEED
        );

        if (result == Z_OK && destLen < fileData.size())
            compressedLen = static_cast<std::uint32_t>(destLen);
    }

    // Update entry
    m_fileEntries[index].dwOffset = m_header.dwEntryOffset;
    m_fileEntries[index].dwLength = static_cast<std::uint32_t>(fileData.size());
    m_fileEntries[index].dwCompressedLength = compressedLen;

    // Write new data
    m_packageFile.seekp(m_header.dwEntryOffset);
    if (compressedLen < m_fileEntries[index].dwLength)
        m_packageFile.write(reinterpret_cast<const char*>(m_compressionBuffer.data()), compressedLen);
    else
        m_packageFile.write(reinterpret_cast<const char*>(fileData.data()), fileData.size());

    m_header.dwEntryOffset += compressedLen;

    m_hasChanged = true;
    m_hasSorted = false;

    return true;
}

bool AFilePackage::ReadFile(std::wstring_view fileName, std::span<std::byte> buffer, std::size_t offset, std::size_t& bytesRead)
{
    AFPCK_FILEENTRY entry;
    if (!GetFileEntry(fileName, entry))
    {
        AFERRLOG(L"AFilePackage::ReadFile(), Can not find file entry [{}]", fileName);
        return false;
    }

    return ReadFile(entry, buffer, offset, bytesRead);
}

bool AFilePackage::ReadFile(const AFPCK_FILEENTRY& entry, std::span<std::byte> buffer, std::size_t offset, std::size_t& bytesRead)
{
    if (offset > entry.dwLength)
    {
        AFERRLOG(L"AFilePackage::ReadFile(), Offset [{}] beyond file length [{}]", offset, entry.dwLength);
        return false;
    }

    std::size_t bytesToRead = entry.dwLength - offset;
    if (buffer.size() < bytesToRead)
    {
        AFERRLOG(L"AFilePackage::ReadFile(), Buffer too small: {} < {}", buffer.size(), bytesToRead);
        return false;
    }

    m_packageFile.seekg(entry.dwOffset + offset);

    if (entry.dwCompressedLength < entry.dwLength)
    {
        if (offset != 0)
        {
            AFERRLOG(L"AFilePackage::ReadFile(), Offset not allowed for compressed files");
            return false;
        }

        m_compressionBuffer.resize(entry.dwCompressedLength);
        m_packageFile.read(reinterpret_cast<char*>(m_compressionBuffer.data()), entry.dwCompressedLength);
        if (m_packageFile.gcount() != static_cast<std::streamsize>(entry.dwCompressedLength))
            return false;

        uLongf destLen = static_cast<uLongf>(bytesToRead);
        int result = uncompress(
            reinterpret_cast<Bytef*>(buffer.data()),
            &destLen,
            reinterpret_cast<const Bytef*>(m_compressionBuffer.data()),
            entry.dwCompressedLength
        );

        if (result != Z_OK)
        {
            AFERRLOG(L"AFilePackage::ReadFile(), Decompression failed: {}", result);
            return false;
        }

        bytesRead = static_cast<std::size_t>(destLen);
    }
    else
    {
        m_packageFile.read(reinterpret_cast<char*>(buffer.data()), bytesToRead);
        bytesRead = static_cast<std::size_t>(m_packageFile.gcount());
    }

    return true;
}

bool AFilePackage::GetFileEntry(std::wstring_view fileName, AFPCK_FILEENTRY& outEntry, int* outIndex) const
{
    auto normalized = NormalizeFileName(fileName);
    if (!m_hasSorted)
    {
        for (size_t i = 0; i < m_fileEntries.size(); ++i)
        {
            if (iequals(normalized, m_fileEntries[i].szFileName))
            {
                outEntry = m_fileEntries[i];
                if (outIndex)
                    *outIndex = static_cast<int>(i);

                return true;
            }
        }
    }
    else
    {
        // Binary search
        auto it = std::lower_bound(m_fileEntries.begin(), m_fileEntries.end(), normalized,
            [](const AFPCK_FILEENTRY& a, const std::string& b) {
                return iequals(a.szFileName, b) ? false : std::strcmp(a.szFileName, b.c_str()) < 0;
            });

        if (it != m_fileEntries.end() && iequals(it->szFileName, normalized))
        {
            outEntry = *it;
            if (outIndex)
                *outIndex = static_cast<int>(it - m_fileEntries.begin());

            return true;
        }
    }

    return false;
}

bool AFilePackage::GetFileEntryByIndex(int index, AFPCK_FILEENTRY& outEntry) const
{
    if (index < 0 || static_cast<size_t>(index) >= m_fileEntries.size())
        return false;

    outEntry = m_fileEntries[index];

    return true;
}

bool AFilePackage::ResortEntries()
{
    std::sort(m_fileEntries.begin(), m_fileEntries.end(),
        [](const AFPCK_FILEENTRY& a, const AFPCK_FILEENTRY& b) {
            return std::strcmp(a.szFileName, b.szFileName) < 0;
        });

    m_hasSorted = true;

    return true;
}

bool AFilePackage::LoadEntries()
{
    // Read file count
    m_packageFile.seekg(-static_cast<std::streamoff>(sizeof(int) + sizeof(std::uint32_t)), std::ios::end);
    int numFiles = 0;
    m_packageFile.read(reinterpret_cast<char*>(&numFiles), sizeof(numFiles));
    if (m_packageFile.fail() || numFiles < 0)
    {
        AFERRLOG(L"AFilePackage::LoadEntries(), Invalid file count");
        return false;
    }

    // Read header
    m_packageFile.seekg(-static_cast<std::streamoff>(sizeof(AFPCK_FILEHEADER) + sizeof(std::uint32_t) + sizeof(int)), std::ios::end);
    m_packageFile.read(reinterpret_cast<char*>(&m_header), sizeof(m_header));
    if (m_packageFile.fail())
    {
        AFERRLOG(L"AFilePackage::LoadEntries(), Failed to read header");
        return false;
    }

    // Load entries
    m_fileEntries.resize(numFiles);
    m_packageFile.seekg(m_header.dwEntryOffset);
    for (int i = 0; i < numFiles; ++i)
    {
        int nameLen = 0;
        m_packageFile.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        if (nameLen <= 0 || nameLen > 260)
        {
            AFERRLOG(L"AFilePackage::LoadEntries(), Invalid filename length: {}", nameLen);
            return false;
        }

        m_packageFile.read(m_fileEntries[i].szFileName, nameLen);
        m_packageFile.read(reinterpret_cast<char*>(&m_fileEntries[i].dwOffset), sizeof(std::uint32_t));
        m_packageFile.read(reinterpret_cast<char*>(&m_fileEntries[i].dwLength), sizeof(std::uint32_t));
        m_packageFile.read(reinterpret_cast<char*>(&m_fileEntries[i].dwCompressedLength), sizeof(std::uint32_t));

        if (m_packageFile.fail())
        {
            AFERRLOG(L"AFilePackage::LoadEntries(), Failed to read entry {}", i);
            return false;
        }
    }

    return true;
}

bool AFilePackage::SaveEntries()
{
    if (m_readOnly)
        return false;

    // Write entries
    m_packageFile.seekp(m_header.dwEntryOffset);
    for (const auto& entry : m_fileEntries)
    {
        int nameLen = static_cast<int>(std::strlen(entry.szFileName)) + 1;
        m_packageFile.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
        m_packageFile.write(entry.szFileName, nameLen);
        m_packageFile.write(reinterpret_cast<const char*>(&entry.dwOffset), sizeof(entry.dwOffset));
        m_packageFile.write(reinterpret_cast<const char*>(&entry.dwLength), sizeof(entry.dwLength));
        m_packageFile.write(reinterpret_cast<const char*>(&entry.dwCompressedLength), sizeof(entry.dwCompressedLength));
    }

    // Write footer: header + count + version
    m_packageFile.write(reinterpret_cast<const char*>(&m_header), sizeof(m_header));
    auto numFiles = static_cast<int>(m_fileEntries.size());
    m_packageFile.write(reinterpret_cast<const char*>(&numFiles), sizeof(numFiles));
    m_packageFile.write(reinterpret_cast<const char*>(&m_header.dwVersion), sizeof(m_header.dwVersion));

    m_hasChanged = false;

    return true;
}

std::string AFilePackage::NormalizeFileName(std::wstring_view fileName) const
{
    std::wstring file(fileName);
    return ::NormalizeFileName(ASTR_UNICODE_TO_UTF8(file));
}

bool OpenFilePackage(std::wstring_view packFile)
{
    CloseFilePackage(); // ensure clean state
    g_globalPackage = std::make_unique<AFilePackage>();

    return g_globalPackage->Open(packFile, AFPCK_OPENMODE::AFPCK_OPENEXIST);
}

bool CloseFilePackage()
{
    if (g_globalPackage)
    {
        bool result = g_globalPackage->Close();
        g_globalPackage.reset();
        return result;
    }

    return true;
}

AFilePackage* GetGlobalFilePackage()
{
    return g_globalPackage.get();
}
