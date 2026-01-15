#include "pch.h"
#include "AFI.h"
#include "ALog.h"
#include "APath.h"

std::wstring g_baseDir;
std::unique_ptr<ALog> g_errorLog;
bool g_compressEnable = false;

bool AFileMod_Initialize(bool bCompressEnable)
{
    AFileMod_Finalize(); // clean up if already initialized

    g_compressEnable = bCompressEnable;

    // Set base dir to current working directory
    g_baseDir = std::filesystem::current_path().wstring();

    // Remove trailing backslash (Windows)
    if (!g_baseDir.empty() && g_baseDir.back() == L'\\')
        g_baseDir.pop_back();

    // Initialize error log
    g_errorLog = std::make_unique<ALog>();
    if (!g_errorLog->Init(L"AF.log", L"Angelica File Module Error Log"))
    {
        g_errorLog.reset();
        return false;
    }

    return true;
}

bool AFileMod_SetBaseDir(std::wstring_view baseDir)
{
    g_baseDir = baseDir;

    // Normalize: remove trailing backslash
    if (!g_baseDir.empty() && g_baseDir.back() == L'\\')
        g_baseDir.pop_back();

    return true;
}

bool AFileMod_Finalize()
{
    if (g_errorLog)
    {
        g_errorLog->Release();
        g_errorLog.reset();
    }

    g_baseDir.clear();

    return true;
}

std::wstring AFileMod_GetBaseDir()
{
    return g_baseDir;
}

// Full path: baseDir + folder + file
std::wstring AFileMod_GetFullPath(std::wstring_view folderName, std::wstring_view fileName)
{
    std::wstring folder(folderName);
    std::wstring file(fileName);
    std::wstring baseWithFolder = APath_GetFullPath(g_baseDir, folder);
    return APath_GetFullPath(baseWithFolder, file);
}

// Full path: baseDir + file
std::wstring AFileMod_GetFullPath(std::wstring_view fileName)
{
    std::wstring file(fileName);
    return APath_GetFullPath(g_baseDir, file);
}

// Relative path: fullPath relative to (baseDir + folder)
std::wstring AFileMod_GetRelativePath(std::wstring_view fullPath, std::wstring_view folderName)
{
    std::wstring folder(folderName);
    std::wstring path(fullPath);
    std::wstring baseWithFolder = APath_GetFullPath(g_baseDir, folder);
    return APath_GetRelativePath(path, baseWithFolder);
}

// Relative path: fullPath relative to baseDir
std::wstring AFileMod_GetRelativePath(std::wstring_view fullPath)
{
    std::wstring path(fullPath);
    return APath_GetRelativePath(path, g_baseDir);
}

// Extract filename (everything after last \ or /)
std::wstring AFileMod_GetFileTitle(std::wstring_view filePath)
{
    if (filePath.empty())
        return {};

    // Find last separator
    auto lastSlash = filePath.find_last_of(L"\\/");
    if (lastSlash == std::wstring_view::npos)
        return std::wstring(filePath);

    return std::wstring(filePath.substr(lastSlash + 1));
}

// Extract path (everything up to last \ or /)
std::wstring AFileMod_GetFilePath(std::wstring_view filePath)
{
    if (filePath.empty())
        return {};

    auto lastSlash = filePath.find_last_of(L"\\/");
    if (lastSlash == std::wstring_view::npos)
        return {}; // no path, just filename

    return std::wstring(filePath.substr(0, lastSlash));
}

ALog* GetAFErrorLog()
{
    return g_errorLog.get();
}

std::wstring_view GetAFBaseDir()
{
    return g_baseDir;
}

bool IsAFCompressionEnabled()
{
    return g_compressEnable;
}
