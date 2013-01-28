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
	dprintf("Current Dashboard: %d.%d.%d.%d\n", XboxKrnlVersion->Major, XboxKrnlVersion->Minor, XboxKrnlVersion->Build, XboxKrnlVersion->Qfe);
}

bool GetCPUKey()
{
	PBYTE buf = (PBYTE)XPhysicalAlloc(0x10, MAXULONG_PTR, 0, MEM_LARGE_PAGES|PAGE_READWRITE|PAGE_NOCACHE);
	if (buf != NULL)
	{
		unsigned long long dest = 0x8000000000000000ULL | ((DWORD)MmGetPhysicalAddress(buf)&0xFFFFFFFF);
		ZeroMemory(buf, 0x10);
		if(HvxPeek(0x20ULL, dest, 0x10ULL) == SYSCALL_KEY)
		{
			memcpy(keybuf, buf, 0x10);
			return true;
		}
	}
	return false;
}

void PrintCPUKey()
{
	dprintf("Your CPUKey is: ");
	int i;
	for (i = 0; i < 0x10; i++)
		dprintf("%02X", keybuf[i]);
	dprintf("\n");
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
		dprintf(" * CPUKey sucessfully saved to: %s\n", file);
	}
}