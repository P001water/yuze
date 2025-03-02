#include <Windows.h>
#include <stdio.h>
#include <time.h>

void dbg_log(const char* format, ...)
{
#ifdef _DEBUG
	CHAR buffer[0x2000];
	CHAR date[0x100];
	CHAR log[0x1f00];

	HANDLE hFile = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwWriteBytes = 0;

	va_list list;
	va_start(list, format);
	wvsprintfA(log, format, list);
	va_end(list);

	SYSTEMTIME st;
	GetLocalTime(&st);
	wsprintfA(date, "[%d/%d/%d %d:%d:%d] ",
		st.wYear,
		st.wMonth,
		st.wDay,
		st.wHour,
		st.wMinute,
		st.wSecond);

	wsprintfA(buffer, "%s %s \r\n", date, log);

	WriteFile(hFile, buffer, strlen(buffer), &dwWriteBytes, 0);
#endif
}