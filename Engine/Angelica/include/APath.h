#ifndef _APATH_H_
#define _APATH_H_

#include <cctype>

inline std::wstring APath_GetRelativePath(const std::wstring& fullPath, const std::wstring& parentPath)
{
    size_t i = 0;
    const size_t minLen = (fullPath.length() < parentPath.length())
        ? fullPath.length()
        : parentPath.length();

    // Case-insensitive comparison (Windows paths are case-insensitive)
    while (i < minLen)
    {
        if (towupper(fullPath[i]) != towupper(parentPath[i]))
            break;

        ++i;
    }

    // If parentPath doesn't match prefix of fullPath, return full path
    if (i < parentPath.length())
        return fullPath;

    // Skip separator after matched part
    if (i < fullPath.length() && fullPath[i] == L'\\')
        ++i;

    return fullPath.substr(i);
}

inline void APath_GetRelativePath(const std::wstring& fullPath, const std::wstring& parentPath, std::wstring& relativePath)
{
    size_t i = 0;
    const size_t minLen = (fullPath.length() < parentPath.length())
        ? fullPath.length()
        : parentPath.length();

    // Case-insensitive prefix match (Windows file system semantics)
    while (i < minLen && towupper(fullPath[i]) == towupper(parentPath[i]))
        ++i;

    if (i < parentPath.length())
    {
        // parentPath is not a prefix → return full path
        relativePath = fullPath;
        return;
    }

    // Skip trailing backslash in parent part
    if (i < fullPath.length() && fullPath[i] == L'\\')
        ++i;

    relativePath = fullPath.substr(i);
}

inline std::wstring APath_GetFullPath(const std::wstring& baseDir, const std::wstring& filename)
{
    if (filename.empty())
        return L"";

    // Absolute path? (e.g., "C:\...")
    if (filename.length() >= 3 && filename[1] == L':' && filename[2] == L'\\')
        return filename;

    // Skip leading ".\"
    std::wstring realFile = filename;
    if (filename.length() >= 2 && filename[0] == L'.' && filename[1] == L'\\')
        realFile = filename.substr(2);

    if (baseDir.empty())
        return realFile;

    std::wstring fullPath = baseDir;
    if (!fullPath.empty() && fullPath.back() != L'\\')
        fullPath += L'\\';

    fullPath += realFile;

    return fullPath;
}

inline void APath_GetFullPath(std::wstring& fullPath, const std::wstring& baseDir, const std::wstring& filename)
{
    fullPath.clear();

    if (filename.empty())
        return;

    // Absolute path? (e.g., "C:\file.txt")
    if (filename.length() >= 3 && filename[1] == L':' && filename[2] == L'\\')
    {
        fullPath = filename;
        return;
    }

    // Strip leading ".\"
    std::wstring realFile = filename;
    if (filename.length() >= 2 && filename[0] == L'.' && filename[1] == L'\\')
        realFile = filename.substr(2);

    if (baseDir.empty())
        fullPath = realFile;
    else
    {
        fullPath = baseDir;
        if (fullPath.back() != L'\\')
            fullPath += L'\\';

        fullPath += realFile;
    }
}

inline std::wstring APath_TrimPath(const std::wstring& path)
{
    if (path.empty())
        return path;

    size_t start = 0;
    size_t end = path.length();

    // Trim leading whitespace
    while (start < end && (path[start] == L' ' || path[start] == L'\t'))
        ++start;

    // Trim trailing whitespace
    while (end > start && (path[end - 1] == L' ' || path[end - 1] == L'\t'))
        --end;

    return path.substr(start, end - start);
}

inline void APath_TrimPath(const std::wstring& path, std::wstring& trimmedPath)
{
    if (path.empty())
    {
        trimmedPath.clear();
        return;
    }

    size_t start = 0;
    size_t end = path.length();

    // Trim leading whitespace
    while (start < end && (path[start] == L' ' || path[start] == L'\t'))
        ++start;

    // Trim trailing whitespace
    while (end > start && (path[end - 1] == L' ' || path[end - 1] == L'\t'))
        --end;

    trimmedPath = path.substr(start, end - start);
}

#endif
