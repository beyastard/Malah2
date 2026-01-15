#ifndef __AFI_H__
#define __AFI_H__

// Initialize file module (enables compression if needed)
bool AFileMod_Initialize(bool bCompressEnable = true);

// Set base directory (relative paths are resolved against this)
bool AFileMod_SetBaseDir(std::wstring_view baseDir);

// Finalize and clean up
bool AFileMod_Finalize();

// Get current base directory
std::wstring AFileMod_GetBaseDir();

// Full path construction
std::wstring AFileMod_GetFullPath(std::wstring_view folderName, std::wstring_view fileName);
std::wstring AFileMod_GetFullPath(std::wstring_view fileName);

// Relative path computation
std::wstring AFileMod_GetRelativePath(std::wstring_view fullPath, std::wstring_view folderName);
std::wstring AFileMod_GetRelativePath(std::wstring_view fullPath);

// Extract file title (filename without path)
std::wstring AFileMod_GetFileTitle(std::wstring_view filePath);

// Extract file path (directory part only)
std::wstring AFileMod_GetFilePath(std::wstring_view filePath);

#endif
