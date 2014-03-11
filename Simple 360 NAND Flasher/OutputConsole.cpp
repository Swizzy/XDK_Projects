#include "AtgConsole.h"
#include "AtgUtil.h"
#include "OutputConsole.h"
#include <stdio.h>

ATG::Console g_console;// console for output
char buf[512]; // Text buffer
#ifdef USE_UNICODE
wchar_t wbuf[1024]; //Unicode Text buffer
#endif
void  __cdecl dprintf(const char* strFormat, ...)
{
	FILE* flog;
	va_list pArglist;
	va_start( pArglist, strFormat );
	vsnprintf_s(buf, 512, strFormat, pArglist);
	va_end( pArglist );
	if (strncmp(buf, MSG_PROCESSING_START, strlen(MSG_PROCESSING_START)) != 0 && strncmp(buf, MSG_PROCESSED_START, strlen(MSG_PROCESSED_START)) != 0)
	{
		fopen_s(&flog, "game:\\Simple 360 NAND Flasher.log", "a");
		if (flog != NULL)
		{
			fprintf(flog, buf);
			fclose(flog);
		}
	}
#ifdef USE_UNICODE
	MultiByteToWideChar(CP_UTF8, 0, buf, 512, wbuf, 1024);
	g_console.Display(wbuf);
#else
	g_console.Display(buf);
#endif
}

void MakeConsole(const char* font, unsigned long BackgroundColor, unsigned long TextColor)
{
	g_console.Create(font, BackgroundColor, TextColor);
	g_console.SendOutputToDebugChannel(TRUE);
}

void ClearConsole()
{
	g_console.Clear();
}