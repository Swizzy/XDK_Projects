#include "AtgConsole.h"
#include "AtgUtil.h"
#include "OutputConsole.h"
#include "Automation.h"
#include <stdio.h>

ATG::Console g_console;// console for output
#ifdef USE_UNICODE
WCHAR buf[512]; // Text buffer
void  __cdecl dprintf(const wchar_t* strFormat, ...)
#else
char buf[512]; // Text buffer
void  __cdecl dprintf(const char* strFormat, ...)
#endif
{
	FILE* flog;
	va_list pArglist;
	va_start(pArglist, strFormat);
#ifdef USE_UNICODE
	_vsnwprintf_s(buf, sizeof(buf), strFormat, pArglist);
#else
	vsnprintf_s(buf, sizeof(buf), strFormat, pArglist);
#endif
	va_end(pArglist);
	g_console.Display(buf);
#ifdef USE_UNICODE
	if (!fexists("game:\\Simple 360 NAND Flasher.log"))
	{
		fopen_s(&flog, "game:\\Simple 360 NAND Flasher.log", "wb");
		char* cStart = "\xfe\xff";
		fwrite(cStart, strlen(cStart), 1, flog);
		fclose(flog);
	}
	if (wcsncmp(strFormat, MSG_PROCESSING_START, wcslen(MSG_PROCESSING_START)) != 0 && wcsncmp(strFormat, MSG_PROCESSED_START, wcslen(MSG_PROCESSED_START)) != 0)
	{
		fopen_s(&flog, "game:\\Simple 360 NAND Flasher.log", "ab");
		if (flog != NULL)
		{
			fwrite(buf, wcslen(buf) * sizeof(wchar_t), 1, flog);
			fclose(flog);
		}
	}
#else
	if (strncmp(buf, MSG_PROCESSING_START, strlen(MSG_PROCESSING_START)) != 0 && strncmp(buf, MSG_PROCESSED_START, strlen(MSG_PROCESSED_START)) != 0)
	{
		fopen_s(&flog, "game:\\Simple 360 NAND Flasher.log", "a");
		if (flog != NULL)
		{
			fprintf(flog, buf);
			fclose(flog);
		}
	}
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
