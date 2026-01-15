#ifndef _AFILEPACKAGE_H_
#define _AFILEPACKAGE_H_

#include <span>

//#define AFPCK_VERSION  0x00010001
//#define AFPCK_VERSION  0x00010002 // Add compression
//#define AFPCK_VERSION  0x00010003 // The final release version on June 2002

struct AFPCK_FILEENTRY
{
	char szFileName[260];             // The file name of this entry; this may contain a path
	std::uint32_t dwOffset;           // The offset from the beginning of the package file
	std::uint32_t dwLength;           // The length of this file
	std::uint32_t dwCompressedLength; // The compressed data length

};

struct AFPCK_FILEHEADER
{
	std::uint32_t dwVersion;     // Composed by two word version, major part and minor part
	std::uint32_t dwEntryOffset; // The entry list offset from the beginning
	char szDescription[256];     // size of array must be 256 bytes
};

enum AFPCK_OPENMODE
{
	AFPCK_OPENEXIST = 0,
	AFPCK_CREATENEW = 1
};

class AFilePackage
{
public:
	AFilePackage() = default;
	~AFilePackage();

	bool Open(std::wstring_view pckPath, AFPCK_OPENMODE mode);
	bool Close();

	bool AppendFile(std::wstring_view fileName, std::span<const std::byte> fileData);
	bool RemoveFile(std::wstring_view fileName);

#ifdef ReplaceFile
#pragma push_macro("ReplaceFile")
#undef ReplaceFile
	bool ReplaceFile(std::wstring_view fileName, std::span<const std::byte> fileData);
#pragma pop_macro("ReplaceFile")
#endif

	bool ReadFile(std::wstring_view fileName, std::span<std::byte> buffer, std::uint32_t offset, std::uint32_t& bytesRead);
	bool ReadFile(const AFPCK_FILEENTRY& entry, std::span<std::byte> buffer, std::uint32_t offset, std::uint32_t& bytesRead);

	bool GetFileEntry(std::wstring_view fileName, AFPCK_FILEENTRY& outEntry, int* outIndex = nullptr) const;
	bool GetFileEntryByIndex(int index, AFPCK_FILEENTRY& outEntry) const;

	bool ResortEntries();

	[[nodiscard]] size_t GetFileNumber() const noexcept { return m_fileEntries.size(); }
	[[nodiscard]] const AFPCK_FILEHEADER& GetFileHeader() const noexcept { return m_header; }

private:
	bool LoadEntries();
	bool SaveEntries();
	std::string NormalizeFileName(std::wstring_view fileName) const;

	std::fstream m_packageFile;
	AFPCK_FILEHEADER m_header{};
	AFPCK_OPENMODE m_mode = AFPCK_OPENMODE::AFPCK_OPENEXIST;
	std::vector<AFPCK_FILEENTRY> m_fileEntries;
	std::vector<std::byte> m_compressionBuffer;

	bool m_hasChanged = false;
	bool m_readOnly = false;
	bool m_hasSorted = false;

	static constexpr std::uint32_t CURRENT_VERSION = 0x00010003u;
};

bool OpenFilePackage(std::wstring_view packFile);
bool CloseFilePackage();
AFilePackage* GetGlobalFilePackage();

#endif
