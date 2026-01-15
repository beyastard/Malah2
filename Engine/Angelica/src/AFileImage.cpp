#include "pch.h"
#include "AFileImage.h"
#include "AFilePackage.h"
#include "AFI.h"
#include "AFPI.h"

AFileImage::~AFileImage()
{
    AFileImage::Close();
}

bool AFileImage::Open(std::wstring_view folderName, std::wstring_view fileName, unsigned int flags)
{
    std::wstring fullPath = AFileMod_GetFullPath(folderName, fileName);
    return Open(fullPath, flags);
}

bool AFileImage::Open(std::wstring_view fullPath, unsigned int flags)
{
    if (m_isOpen)
        Close();

    if (!Init(fullPath))
    {
        AFERRLOG(L"AFileImage::Open(), Can not init the file image!");
        return false;
    }

    if (!(flags & AFILE_OPENEXIST))
    {
        AFERRLOG(L"AFileImage::Open() Currently only supports read flag for file images");
        return false;
    }

    // Read FOURCC header
    if (m_fileImage.size() < 4)
    {
        m_flags = flags | AFILE_TEXT; // Default to text for empty/short files
        m_isOpen = true;
        return true;
    }

    unsigned int fourcc = 0;
    std::memcpy(&fourcc, m_fileImage.data(), sizeof(fourcc));
    m_flags = flags & ~(AFILE_BINARY | AFILE_TEXT);

    constexpr unsigned int BINARY_FOURCC = 0x42584f4du; // 'MOXB'
    constexpr unsigned int TEXT_FOURCC = 0x54584f4du;   // 'MOXT'

    if (fourcc == BINARY_FOURCC)
        m_flags |= AFILE_BINARY;
    else if (fourcc == TEXT_FOURCC)
        m_flags |= AFILE_TEXT;
    else
    {
        // No valid FOURCC → treat as text, don't skip header
        m_flags |= AFILE_TEXT;
        m_currentPos = 0;
    }

    m_isOpen = true;

    return true;
}

bool AFileImage::ResetPointer()
{
    return FImgSeek(0, std::ios::beg);
}

bool AFileImage::Close()
{
    m_currentPos = 0;
    m_fileImage.clear();
    m_isOpen = false;

    return true;
}

bool AFileImage::Read(void* buffer, size_t bufferLength, size_t& bytesRead)
{
    if (!m_isOpen || !buffer)
    {
        bytesRead = 0;
        return false;
    }

    return FImgRead(static_cast<std::byte*>(buffer), bufferLength, bytesRead);
}

bool AFileImage::Write(const void* buffer, size_t bufferLength, size_t& bytesWritten)
{
    bytesWritten = 0;
    return false;
}

bool AFileImage::ReadLine(std::string& line, size_t maxLineLength)
{
    if (!m_isOpen)
        return false;

    return FImgReadLine(line, maxLineLength);
}

bool AFileImage::ReadString(std::string& str)
{
    if (!m_isOpen)
        return false;

    str.clear();

    char ch = '\0';
    size_t bytesRead = 0;

    while (Read(&ch, 1, bytesRead) && ch != '\0')
        str += ch;

    return true;
}

bool AFileImage::WriteLine(std::string_view line)
{
    return false;
}

size_t AFileImage::GetPos()
{
    return m_currentPos;
}

bool AFileImage::Seek(size_t offset, std::ios::seekdir origin)
{
    return FImgSeek(offset, origin);
}

bool AFileImage::Init(std::wstring_view fullPath)
{
    m_fileName = fullPath;
    m_relativeName = AFileMod_GetRelativePath(fullPath);

    // Try to load from global package first (legacy compatibility)
    extern AFilePackage* g_pAFilePackage;
    if (g_pAFilePackage)
    {
        AFPCK_FILEENTRY entry;
        if (g_pAFilePackage->GetFileEntry(m_relativeName, entry))
        {
            m_fileImage.resize(entry.dwLength);
            size_t bytesRead = 0;
            if (!g_pAFilePackage->ReadFile(entry, std::span<std::byte>(reinterpret_cast<std::byte*>(
                m_fileImage.data()), m_fileImage.size()), 0, bytesRead))
            {
                AFERRLOG(L"AFileImage::Init(), Error reading file [{}] from package!", m_relativeName);
                return false;
            }

            return true;
        }
    }

    // Load from filesystem
    std::wstring fullPathStr(fullPath);
    std::ifstream file(fullPathStr, std::ios::binary);
    if (!file)
    {
        AFERRLOG(L"AFileImage::Init() Can't open file [{}] to create image in memory", fullPath);
        return false;
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = static_cast<size_t>(file.tellg());
    if (fileSize == 0)
    {
        AFERRLOG(L"AFileImage::Init() The file [{}] is zero length!", fullPath);
        return false;
    }

    m_fileImage.resize(fileSize);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(m_fileImage.data()), fileSize);

    if (file.gcount() != static_cast<std::streamsize>(fileSize))
    {
        AFERRLOG(L"AFileImage::Init() Failed to read entire file [{}]", fullPath);
        return false;
    }

    return true;
}

bool AFileImage::Release()
{
    m_fileImage.clear();
    m_currentPos = 0;
    return true;
}

bool AFileImage::FImgRead(std::byte* buffer, size_t size, size_t& bytesRead)
{
    bytesRead = 0;
    if (m_currentPos >= m_fileImage.size())
        return true; // EOF

    size_t available = m_fileImage.size() - m_currentPos;
    size_t toRead = std::min(size, available);

    if (toRead > 0)
    {
        std::memcpy(buffer, m_fileImage.data() + m_currentPos, toRead);
        m_currentPos += toRead;
        bytesRead = toRead;
    }

    return true;
}

bool AFileImage::FImgReadLine(std::string& line, size_t maxLineLength)
{
    line.clear();
    size_t startPos = m_currentPos;

    while (m_currentPos < m_fileImage.size())
    {
        std::byte byte = m_fileImage[m_currentPos];
        char ch = static_cast<char>(byte);

        if (ch == '\n' || ch == '\r')
        {
            // Handle line ending
            m_currentPos++; // consume the \n or \r

            // Check for \r\n sequence
            if (ch == '\r' && m_currentPos < m_fileImage.size() && static_cast<char>(m_fileImage[m_currentPos]) == '\n')
                m_currentPos++; // consume the \n

            break;
        }

        line += ch;
        m_currentPos++;

        if (line.size() >= maxLineLength)
            break;
    }

    return !line.empty() || (m_currentPos > startPos);
}

bool AFileImage::FImgSeek(size_t offset, std::ios::seekdir origin)
{
    size_t newPos = 0;

    switch (origin)
    {
    case std::ios::beg:
        newPos = offset;
        break;
    case std::ios::cur:
        newPos = m_currentPos + offset;
        break;
    case std::ios::end:
        newPos = m_fileImage.size() + offset;
        break;
    default:
        return false;
    }

    // Clamp to valid range [0, fileSize]
    m_currentPos = std::min(newPos, m_fileImage.size());

    return true;
}
