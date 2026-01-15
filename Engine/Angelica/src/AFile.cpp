#include "pch.h"
#include "AFile.h"
#include "AFI.h"
#include "AFPI.h"

#include <cstring>
#include <system_error>

AFile::AFile()
{}

AFile::~AFile()
{
	AFile::Close();
}

bool AFile::Open(std::wstring_view fullPath, std::uint32_t flags)
{
    if (m_isOpen)
        Close();

    m_fileName = fullPath;
    m_relativeName = AFileMod_GetRelativePath(fullPath);

    // Determine open mode
    std::ios::openmode mode = std::ios::binary; // always start with binary

    if (flags & AFILE_CREATENEW)
        mode |= std::ios::out | std::ios::trunc;
    else if (flags & AFILE_OPENAPPEND)
        mode |= std::ios::in | std::ios::out | std::ios::app;
    else
        mode |= std::ios::in; // AFILE_OPENEXIST (or default)

    m_file.open(m_fileName, mode);
    if (!m_file)
    {
        AFERRLOG(L"Failed to open file: {}", m_fileName);
        return false;
    }

    // Handle FOURCC header
    constexpr std::uint32_t BINARY_FOURCC = AFILE_TYPE_BINARY; // 'MOXB'
    constexpr std::uint32_t TEXT_FOURCC = AFILE_TYPE_TEXT;     // 'MOXT'

    if (flags & AFILE_CREATENEW)
    {
        m_flags = flags;
        std::uint32_t fourcc = IsText() ? TEXT_FOURCC : BINARY_FOURCC;
        m_file.write(reinterpret_cast<const char*>(&fourcc), sizeof(fourcc));
    }
    else
    {
        // Read existing header
        m_flags = flags & ~(AFILE_BINARY | AFILE_TEXT);

        std::uint32_t fourcc = 0;
        m_file.read(reinterpret_cast<char*>(&fourcc), sizeof(fourcc));

        if (m_file.gcount() == sizeof(fourcc))
        {
            if (fourcc == BINARY_FOURCC)
                m_flags |= AFILE_BINARY;
            else if (fourcc == TEXT_FOURCC)
                m_flags |= AFILE_TEXT;
            else
            {
                // No valid FOURCC → treat as text, rewind
                m_flags |= AFILE_TEXT;
                m_file.seekg(0);
            }
        }
        else
        {
            // Empty or short file → treat as text
            m_flags |= AFILE_TEXT;
            m_file.clear(); // clear failbit
            m_file.seekg(0);
        }
    }

    m_isOpen = true;

    return true;
}

bool AFile::Open(std::wstring_view folderName, std::wstring_view fileName, std::uint32_t flags)
{
    std::wstring folder(folderName);
    std::wstring file(fileName);
    std::wstring fullPath = AFileMod_GetFullPath(folder, file);
    return Open(fullPath, flags);
}

bool AFile::ResetPointer()
{
    return Seek(0, AFILE_SEEK_SET);
}

bool AFile::Close()
{
    if (m_file.is_open())
        m_file.close();

    m_isOpen = false;

    return true;
}

bool AFile::Read(void* buffer, size_t bufferLength, size_t& bytesRead)
{
    if (!m_isOpen || !buffer || bufferLength == 0)
    {
        bytesRead = 0;
        return false;
    }

    m_file.read(static_cast<char*>(buffer), static_cast<std::streamsize>(bufferLength));
    bytesRead = static_cast<size_t>(m_file.gcount());

    return true;
}

bool AFile::Write(const void* buffer, size_t bufferLength, size_t& bytesWritten)
{
    if (!m_isOpen || !buffer || bufferLength == 0)
    {
        bytesWritten = 0;
        return false;
    }

    m_file.write(static_cast<const char*>(buffer), static_cast<std::streamsize>(bufferLength));
    bytesWritten = bufferLength;

    return m_file.good();
}

bool AFile::ReadLine(std::string& line, size_t maxLineLength)
{
    if (!m_isOpen || !IsText())
        return false;


    line.clear();
    std::getline(m_file, line);

    if (m_file.fail() && !m_file.eof())
        return false;


    // Trim \r (Windows line endings)
    if (!line.empty() && line.back() == '\r')
        line.pop_back();


    // Enforce max length (optional)
    if (line.size() > maxLineLength)
        line.resize(maxLineLength);

    return true;
}

bool AFile::ReadString(std::string& str)
{
    if (!m_isOpen)
        return false;

    str.clear();
    char ch;
    while (m_file.get(ch) && ch != '\0')
        str += ch;

    return true;
}

bool AFile::WriteLine(std::string_view line)
{
    if (!m_isOpen || !IsText())
        return false;
    
    m_file << line << '\n';

    return m_file.good();
}

bool AFile::GetStringAfter(std::string_view buffer, std::string_view tag, std::string& result)
{
    result.clear();

    if (buffer.empty() || tag.empty())
        return false;

    // Only match if tag appears at start of buffer
    if (buffer.substr(0, tag.size()) != tag)
        return false;

    result = std::string(buffer.substr(tag.size()));

    return true;
}

size_t AFile::GetPos()
{
    if (!m_isOpen)
        return 0;

    return static_cast<size_t>(m_file.tellg());
}

bool AFile::Seek(size_t offset, std::ios::seekdir origin)
{
    if (!m_isOpen)
        return false;

    m_file.seekg(static_cast<std::streamoff>(offset), origin);

    return m_file.good();
}
