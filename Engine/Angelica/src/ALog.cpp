#include "pch.h"
#include "ALog.h"
#include "ATime.h"
#include "APath.h"

#include <io.h>
#include <direct.h>

// Helper: Create directory recursively (simple version for single-level since we're only create the 'Logs' directory)
void SafeCreateDirW(const std::wstring& wstrDir)
{
    if (_waccess(wstrDir.c_str(), 0) == -1)
        (void)_wmkdir(wstrDir.c_str()); // Note: _wmkdir only creates one level; for deep paths, use SHCreateDirectory or recursive implementtation
}

std::wstring ALog::m_wstrLogDir = L"Logs";

ALog::ALog()
    : m_pFile(nullptr)
{}

ALog::~ALog()
{
    if (m_pFile)
    {
        fclose(m_pFile);
        m_pFile = nullptr;
    }
}

bool ALog::Init(const std::wstring& logFile, const std::wstring& helloMsg, bool bAppend)
{
    WORD nYear, nMonth, nDay, nDate, nHour, nMinute, nSecond;
    std::wstring wstrLogPath = m_wstrLogDir + L"\\" + logFile;

    SafeCreateDirW(m_wstrLogDir);

    m_pFile = _wfopen(wstrLogPath.c_str(), bAppend ? L"at, ccs=UTF-16LE" : L"wt, ccs=UTF-16LE");
    // Alternative: use L"wt" without BOM if you prefer plain UTF-16LE without BOM.
    // Or use L"wt,ccs=UNICODE" (same as UTF-16LE with BOM on Windows)

    if (!m_pFile)
        return false; // Note: changed from 'true' — returning false on failure is more conventional

    ATime_GetSystemTime(&nYear, &nMonth, &nDay, &nDate, &nHour, &nMinute, &nSecond);
    fwprintf(m_pFile, L"%ls\nCreated(or opened) on %02d/%02d/%04d %02d:%02d:%02d\n\n",
        helloMsg.c_str(), nDay, nMonth, nYear, nHour, nMinute, nSecond);
    fflush(m_pFile);

    return true;
}

bool ALog::Release()
{
    LogString(L"Log file closed successfully!");

    if (m_pFile)
    {
        fclose(m_pFile);
        m_pFile = nullptr;
    }

    return true;
}

bool ALog::Log(const wchar_t* fmt, ...)
{
    if (!m_pFile || !fmt)
        return false;

    wchar_t szBuffer[2048];
    va_list args;
    va_start(args, fmt);
    int result = _vsnwprintf(szBuffer, _countof(szBuffer) - 1, fmt, args);
    va_end(args);

    if (result < 0)
        szBuffer[_countof(szBuffer) - 1] = L'\0'; // Ensure null termination

    return LogString(std::wstring(szBuffer));
}

bool ALog::LogString(const std::wstring& str)
{
    if (!m_pFile) return false;

    WORD nYear, nMonth, nDay, nDate, nHour, nMinute, nSecond;
    ATime_GetSystemTime(&nYear, &nMonth, &nDay, &nDate, &nHour, &nMinute, &nSecond);

    fwprintf(m_pFile, L"[%02d:%02d:%02d] %ls\n", nHour, nMinute, nSecond, str.c_str());
    fflush(m_pFile);

    return true;
}

void ALog::SetLogDir(const std::wstring& logDir)
{
    m_wstrLogDir = logDir;

    // Remove trailing backslash if present
    if (!m_wstrLogDir.empty() && m_wstrLogDir.back() == L'\\')
        m_wstrLogDir.pop_back();

    SafeCreateDirW(m_wstrLogDir);
}
