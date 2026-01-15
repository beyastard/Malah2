#ifndef __AFILE_H__
#define __AFILE_H__

// Flags
constexpr std::uint32_t AFILE_TYPE_BINARY = 0x42584f4du;
constexpr std::uint32_t AFILE_TYPE_TEXT = 0x54584f4du;

constexpr std::uint32_t AFILE_OPENEXIST = 0x00000001u;
constexpr std::uint32_t AFILE_CREATENEW = 0x00000002u;
constexpr std::uint32_t AFILE_OPENAPPEND = 0x00000004u;
constexpr std::uint32_t AFILE_TEXT = 0x00000008u;
constexpr std::uint32_t AFILE_BINARY = 0x00000010u;

constexpr size_t AFILE_LINEMAXLEN = 2048;

// Seek origins (map to std::ios)
constexpr auto AFILE_SEEK_SET = std::ios::beg;
constexpr auto AFILE_SEEK_CUR = std::ios::cur;
constexpr auto AFILE_SEEK_END = std::ios::end;

class AFile
{
public:
    AFile();
    virtual ~AFile();

    // Open with full path
    virtual bool Open(std::wstring_view fullPath, std::uint32_t flags);

    // Open with folder + filename (resolves via AFI)
    virtual bool Open(std::wstring_view folderName, std::wstring_view fileName, std::uint32_t flags);

    virtual bool ResetPointer();
    virtual bool Close();

    // Binary I/O
    virtual bool Read(void* buffer, size_t bufferLength, size_t& bytesRead);
    virtual bool Write(const void* buffer, size_t bufferLength, size_t& bytesWritten);

    // Text I/O
    virtual bool ReadLine(std::string& line, size_t maxLineLength = AFILE_LINEMAXLEN);
    virtual bool ReadString(std::string& str); // null-terminated string
    virtual bool WriteLine(std::string_view line);

    // Utility
    virtual bool GetStringAfter(std::string_view buffer, std::string_view tag, std::string& result);
    virtual size_t GetPos();
    virtual bool Seek(size_t offset, std::ios::seekdir origin);

    // Accessors
    [[nodiscard]] std::uint32_t GetFlags() const noexcept { return m_flags; }
    [[nodiscard]] bool IsBinary() const noexcept { return !IsText(); }
    [[nodiscard]] bool IsText() const noexcept { return (m_flags & AFILE_TEXT) != 0; }
    [[nodiscard]] const std::wstring& GetFileName() const noexcept { return m_fileName; }
    [[nodiscard]] const std::wstring& GetRelativeName() const noexcept { return m_relativeName; }

protected:
    std::wstring m_fileName;     // full path
    std::wstring m_relativeName; // relative to base dir
    std::uint32_t m_flags = 0;
    bool m_isOpen = false;
    std::fstream m_file;
};

#endif
