#include "AtgConsole.h"
#include "AtgUtil.h"
#include "OutputConsole.h"
#include <stdio.h>

ATG::Console g_console;// console for output
char buf[512]; //Text buffer
void  __cdecl dprintf(const char* strFormat, ...)
{
	FILE* flog;
	va_list pArglist;
	va_start( pArglist, strFormat );
	vsnprintf_s(buf, 512, strFormat, pArglist);
	va_end( pArglist );
	const CHAR* ignore = "\r";
	if (strncmp(strFormat, ignore, strlen(ignore)) != 0)
	{
		fopen_s(&flog, "game:\\Simple 360 NAND Flasher.log", "a");
		if (flog != NULL)
		{
			fprintf(flog, buf);
			fclose(flog);
		}
	}
	g_console.Display(buf);
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