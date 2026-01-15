#ifndef _ALOG_H_
#define _ALOG_H_

#include <cstdarg>

// USAGE EXAMPLES:
//
//   #include "ALog.h"
//   #include "AStringConv.h" // if converting from UTF-8/CP936
//
//   // Option 1: Native wide strings
//   ALog log;
//   log.Init(L"MyApp.log", L"Hello from Unicode logger!", false);
//   log.Log(L"User %ls logged in at %d", L"杜宇欣", 123);
//   log.Release();
//
//   // Option 2: Convert from CP936
//   std::string gbkMsg = "\xB2\xE2\xCA\xD4"; // "测试"
//   std::wstring wMsg = AString_CP936ToUnicode(gbkMsg);
//   log.Init(L"test.log", wMsg);

class ALog
{
public:
    ALog();
    ~ALog();

    // Init a log file
    //   logFile     – log filename (relative to log dir)
    //   helloMsg    – initial message
    //   bAppend     – append mode flag
    bool Init(const std::wstring& logFile, const std::wstring& helloMsg, bool bAppend = false);

    // Close log file and write final message
    bool Release();

    // Log formatted message (wide string format)
    bool Log(const wchar_t* fmt, ...);

    // Log raw wide string
    bool LogString(const std::wstring& str);

    // Static log directory control
    static void SetLogDir(const std::wstring& logDir);
    static const std::wstring& GetLogDir() { return m_wstrLogDir; }

private:
    FILE* m_pFile;
    static std::wstring m_wstrLogDir;
};

typedef ALog* PALog;

#endif
