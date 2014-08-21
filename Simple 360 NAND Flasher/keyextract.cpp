#include "keyextract.h"
#include <stdio.h>
#include "OutputConsole.h"

unsigned char keybuf[0x10];

unsigned long long __declspec(naked) HvxPeekCall(DWORD key, unsigned long long type, unsigned long long SourceAddress, unsigned long long DestAddress, unsigned long long lenInBytes)
{ 
	__asm {
		li      r0, 0x0
		sc
		blr
	}
}

unsigned long long HvxPeek(unsigned long long SourceAddress, unsigned long long DestAddress, unsigned long long lenInBytes)
{
	return HvxPeekCall(SYSCALL_KEY, 5, SourceAddress, DestAddress, lenInBytes);
}

void PrintDash()
{
	dprintf(MSG_CURRENT_DASHBOARD, XboxKrnlVersion->Major, XboxKrnlVersion->Minor, XboxKrnlVersion->Build, XboxKrnlVersion->Qfe);
}

PVOID resolveFunction(PCHAR szModuleName, DWORD dwOrdinal)
{
	PVOID pProc = NULL;
	HANDLE hModuleHandle;
	if(NT_SUCCESS(XexGetModuleHandle(szModuleName, &hModuleHandle)))
		XexGetProcedureAddress(hModuleHandle, dwOrdinal, &pProc);
	return pProc;
}

pldata ldat;

void PrintDLVersion()
{
	ldat = (pldata)resolveFunction(MODULE_LAUNCH, DL_ORDINALS_LDAT);
	if (ldat != NULL)
		dprintf(MSG_CURRENT_DASHLAUNCH, ldat->versionMaj, ldat->versionMin, ldat->svnVer);
	else
		dprintf(MSG_DASHLAUNCH_NOT_RUNNING);
}

bool GetCPUKey()
{
	PBYTE buf = (PBYTE)XPhysicalAlloc(0x10, MAXULONG_PTR, 0, MEM_LARGE_PAGES|PAGE_READWRITE|PAGE_NOCACHE);
	if (buf != NULL)
	{
		unsigned long long dest = 0x8000000000000000ULL | ((DWORD)MmGetPhysicalAddress(buf)&0xFFFFFFFF);
		ZeroMemory(buf, 0x10);
		unsigned long long ret = HvxPeek(0x20ULL, dest, 0x10ULL);
		if(ret == SYSCALL_KEY || ret == dest)
		{
			memcpy(keybuf, buf, 0x10);
			return true;
		}
		else
			dprintf(MSG_SYSCALL_RETURN_VALUE, ret);
	}
	return false;
}

void PrintCPUKey()
{
	dprintf(MSG_YOUR_CPUKEY_IS);
	int i;
	for (i = 0; i < 0x10; i++)
#ifdef USE_UNICODE
		dprintf(L"%02X", keybuf[i]);
#else
		dprintf("%02X", keybuf[i]);
#endif
#ifdef USE_UNICODE
	dprintf(L"\n");
#else
	dprintf("\n");
#endif
}

void SaveCPUKey(const char* file)
{
	FILE* fd;
	fopen_s(&fd, file, "w");
	if (fd != NULL)
	{
		int i;
		for (i = 0; i < 0x10; i++)
			fprintf(fd, "%02X", keybuf[i]);
		fclose(fd);
		dprintf(MSG_CPUKEY_SUCCESSFULLY_SAVED, file);
	}
}