#ifndef __ASTRINGCONV_H__
#define __ASTRINGCONV_H__

#include <cstddef>

// ------------------------------------------------------------
// Core: std::string <-> std::wstring (via code page)
// ------------------------------------------------------------

inline std::wstring AString_ToWString(const std::string& str, UINT codePage = CP_ACP)
{
    if (str.empty())
        return std::wstring();

    int len = MultiByteToWideChar(codePage, 0, str.c_str(), -1, NULL, 0);
    if (len <= 1)
        return std::wstring();

    std::wstring wstr(len - 1, L'\0');
    MultiByteToWideChar(codePage, 0, str.c_str(), -1, &wstr[0], len);

    return wstr;
}

inline std::string AString_FromWString(const std::wstring& wstr, UINT codePage = CP_ACP)
{
    if (wstr.empty())
        return std::string();

    int len = WideCharToMultiByte(codePage, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (len <= 1)
        return std::string();

    std::string str(len - 1, '\0');
    WideCharToMultiByte(codePage, 0, wstr.c_str(), -1, &str[0], len, NULL, NULL);

    return str;
}

// ------------------------------------------------------------
// Overloads: const char* (null-terminated) -> std::wstring
// ------------------------------------------------------------

inline std::wstring AString_ToWString(const char* str, UINT codePage = CP_ACP)
{
    if (!str || !*str)
        return std::wstring();

    int len = MultiByteToWideChar(codePage, 0, str, -1, NULL, 0);
    if (len <= 1)
        return std::wstring();

    std::wstring wstr(len - 1, L'\0');
    MultiByteToWideChar(codePage, 0, str, -1, &wstr[0], len);

    return wstr;
}

// Overload: const char* + explicit length (non-null-terminated safe)
inline std::wstring AString_ToWString(const char* str, size_t len, UINT codePage)
{
    if (!str || len == 0)
        return std::wstring();

    int wideLen = MultiByteToWideChar(codePage, 0, str, static_cast<int>(len), NULL, 0);
    if (wideLen == 0)
        return std::wstring();

    std::wstring wstr(wideLen, L'\0');
    MultiByteToWideChar(codePage, 0, str, static_cast<int>(len), &wstr[0], wideLen);

    return wstr;
}

// ------------------------------------------------------------
// Overloads: const wchar_t* (null-terminated) -> std::string
// ------------------------------------------------------------

inline std::string AString_FromWString(const wchar_t* wstr, UINT codePage = CP_ACP)
{
    if (!wstr || !*wstr)
        return std::string();

    int len = WideCharToMultiByte(codePage, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (len <= 1)
        return std::string();

    std::string str(len - 1, '\0');
    WideCharToMultiByte(codePage, 0, wstr, -1, &str[0], len, NULL, NULL);

    return str;
}

// Overload: const wchar_t* + explicit length
inline std::string AString_FromWString(const wchar_t* wstr, size_t len, UINT codePage)
{
    if (!wstr || len == 0)
        return std::string();

    int mbLen = WideCharToMultiByte(codePage, 0, wstr, static_cast<int>(len), NULL, 0, NULL, NULL);
    if (mbLen == 0)
        return std::string();

    std::string str(mbLen, '\0');
    WideCharToMultiByte(codePage, 0, wstr, static_cast<int>(len), &str[0], mbLen, NULL, NULL);

    return str;
}

// ------------------------------------------------------------
// Two-parameter versions (input, output) – for all major combos
// ------------------------------------------------------------

// char* → std::wstring
inline void AString_ToWString(const char* input, std::wstring& output, UINT codePage = CP_ACP)
{
    output = AString_ToWString(input, codePage);
}

// std::string → std::wstring
inline void AString_ToWString(const std::string& input, std::wstring& output, UINT codePage = CP_ACP)
{
    output = AString_ToWString(input, codePage);
}

// wchar_t* → std::string
inline void AString_FromWString(const wchar_t* input, std::string& output, UINT codePage = CP_ACP)
{
    output = AString_FromWString(input, codePage);
}

// std::wstring → std::string
inline void AString_FromWString(const std::wstring& input, std::string& output, UINT codePage = CP_ACP)
{
    output = AString_FromWString(input, codePage);
}

// ------------------------------------------------------------
// Convenience aliases for common encodings
// ------------------------------------------------------------

// UTF-8
inline std::wstring AString_UTF8ToUnicode(const char* utf8) { return AString_ToWString(utf8, CP_UTF8); }
inline std::wstring AString_UTF8ToUnicode(const std::string& utf8) { return AString_ToWString(utf8, CP_UTF8); }
inline std::string AString_UnicodeToUTF8(const wchar_t* unicode) { return AString_FromWString(unicode, CP_UTF8); }
inline std::string AString_UnicodeToUTF8(const std::wstring& unicode) { return AString_FromWString(unicode, CP_UTF8); }

// CP936 (GBK)
inline std::wstring AString_CP936ToUnicode(const char* gbk) { return AString_ToWString(gbk, 936); }
inline std::wstring AString_CP936ToUnicode(const std::string& gbk) { return AString_ToWString(gbk, 936); }
inline std::string AString_UnicodeToCP936(const wchar_t* unicode) { return AString_FromWString(unicode, 936); }
inline std::string AString_UnicodeToCP936(const std::wstring& unicode) { return AString_FromWString(unicode, 936); }

// ANSI (system default)
inline std::wstring AString_ANSIToUnicode(const char* ansi) { return AString_ToWString(ansi, CP_ACP); }
inline std::string AString_UnicodeToANSI(const wchar_t* unicode) { return AString_FromWString(unicode, CP_ACP); }

// ------------------------------------------------------------
// Optional #define aliases (if you prefer macro shorthand)
// ------------------------------------------------------------

// These mimic "function-like" macros but delegate to real functions
#define ASTR_ANSI_TO_UNICODE(str)  AString_ToWString((str), CP_ACP)
#define ASTR_UTF8_TO_UNICODE(str)  AString_ToWString((str), CP_UTF8)
#define ASTR_CP936_TO_UNICODE(str) AString_ToWString((str), 936)
#define ASTR_UNICODE_TO_UTF8(str)  AString_FromWString((str), CP_UTF8)
#define ASTR_UNICODE_TO_CP936(str) AString_FromWString((str), 936)
#define ASTR_UNICODE_TO_ANSI(str)  AString_FromWString((str), CP_ACP)

// Example usages:
//
//   std::wstring w = ASTR_CP936_TO_UNICODE("测试.txt");
//
// Legacy C string → Unicode (for CreateFileW)
//   const char* gbkName = "\xB2\xE2\xCA\xD4.txt"; // "测试.txt" in GBK
//   std::wstring wname = ASTR_CP936_TO_UNICODE(gbkName);
//   HANDLE h = CreateFileW(wname.c_str(), ...);
//
// Or without macro:
//   std::wstring w2 = AString_CP936ToUnicode(gbkName);
//
// Out-param style (matches your path function pattern):
//   std::wstring result;
//   AString_ToWString(gbkName, result, 936);
//
// Convert back to UTF-8 for saving:
//   std::string utf8 = ASTR_UNICODE_TO_UTF8(wname);

#endif
