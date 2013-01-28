#pragma once
#include <xtl.h>

#define SYSCALL_KEY	0x72627472

extern "C" NTSYSAPI PVOID NTAPI MmGetPhysicalAddress(IN PVOID Address);

typedef struct _XBOX_KRNL_VERSION{
	WORD Major; // for 360 this is always 2
	WORD Minor; // usually 0
	WORD Build; // current version, for example 16202
	WORD Qfe; // usually 0
} XBOX_KRNL_VERSION, *PXBOX_KRNL_VERSION;

extern "C" PXBOX_KRNL_VERSION XboxKrnlVersion;

void PrintDash();

bool GetCPUKey();

void PrintCPUKey();

void SaveCPUKey(const char* file);