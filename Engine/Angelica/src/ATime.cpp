#include "pch.h"
#include "ATime.h"

#include <mmsystem.h> // need to include "winmm.lib"

void ATime_GetSystemTime(WORD* pYear, WORD* pMonth, WORD* pDay, WORD* pDate, WORD* pHour, WORD* pMinute, WORD* pSecond)
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	if (pYear)   *pYear   = st.wYear;
	if (pMonth)  *pMonth  = st.wMonth;
	if (pDay)    *pDay    = st.wDay;
	if (pDate)   *pDate   = st.wDayOfWeek;
	if (pHour)   *pHour   = st.wHour;
	if (pMinute) *pMinute = st.wMinute;
	if (pSecond) *pSecond = st.wSecond;
}

DWORD ATime_GetTime()
{
	return timeGetTime();
}
