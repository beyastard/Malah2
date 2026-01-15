#ifndef _AFILEIMAGE_H_
#define _AFILEIMAGE_H_

#include "AFile.h"

class AFileImage : public AFile
{
public:
    AFileImage() = default;
    ~AFileImage();

    bool Open(std::wstring_view folderName, std::wstring_view fileName, unsigned int flags) override;
    bool Open(std::wstring_view fullPath, unsigned int flags) override;
    bool ResetPointer() override;
    bool Close() override;

    bool Read(void* buffer, size_t bufferLength, size_t& bytesRead) override;
    bool Write(const void* buffer, size_t bufferLength, size_t& bytesWritten) override;

    bool ReadLine(std::string& line, size_t maxLineLength = AFILE_LINEMAXLEN) override;
    bool ReadString(std::string& str) override;
    bool WriteLine(std::string_view line) override;

    size_t GetPos() override;
    bool Seek(size_t offset, std::ios::seekdir origin) override;

    // Accessors (modernized)
    [[nodiscard]] const std::vector<std::byte>& GetFileBuffer() const noexcept { return m_fileImage; }
    [[nodiscard]] size_t GetFileLength() const noexcept { return m_fileImage.size(); }

protected:
    bool Init(std::wstring_view fullPath);
    bool Release();

private:
    bool FImgRead(std::byte* buffer, size_t size, size_t& bytesRead);
    bool FImgReadLine(std::string& line, size_t maxLineLength);
    bool FImgSeek(size_t offset, std::ios::seekdir origin);

    std::vector<std::byte> m_fileImage;
    size_t m_currentPos = 0;
};

#endif
