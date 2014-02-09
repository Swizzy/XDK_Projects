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

typedef LONG NTSTATUS;

typedef enum {
	DL_ORDINALS_LDAT = 1,
	DL_ORDINALS_STARTSYSMOD,
	DL_ORDINALS_SHUTDOWN,
	DL_ORDINALS_FORCEINILOAD,
	DL_ORDINALS_GETNUMOPTS,
	DL_ORDINALS_GETOPTINFO,
	DL_ORDINALS_GETOPTVAL,
	DL_ORDINALS_SETOPTVAL,
	DL_ORDINALS_GETOPTVALBYNAME,
	DL_ORDINALS_SETOPTVALBYNAME,
	DL_ORDINALS_GETDRIVELIST,
	DL_ORDINALS_GETDRIVEINFO,
} DL_ORDINALS;

typedef struct _ldata{
	DWORD ID;
	DWORD ltype;
	char link[MAX_PATH];
	char dev[MAX_PATH];
	USHORT versionMaj;
	USHORT versionMin;
	USHORT targetKernel;
	USHORT svnVer;
	DWORD options; // for external apps that want to know what dash launch has set/parsed
	DWORD DebugRoutine; // for external apps that want to recursively hook and call the first/last chance exception trap on their own
	DWORD DebugStepPatch; // address to path single step exception to not be skipped (write 0x60000000/nop to this address to enable it)
	PBYTE tempData; // DL will monitor temps, a copy of the smc temp data is placed here, 0x10 bytes in len
	DWORD iniPathSel; // the path corresponding to this number can be gotten via dlaunchGetDriveList, 0xFF is none, 0xFE is forced
} ldata, *pldata;

#ifdef __cplusplus
extern "C" {
#endif
	NTSTATUS XexGetModuleHandle(
		IN		PSZ moduleName,
		IN OUT	PHANDLE hand
	); 

	NTSTATUS XexGetProcedureAddress(
		IN		HANDLE hand,
		IN		DWORD dwOrdinal,
		IN		PVOID Address
	);
#ifdef __cplusplus
}
#endif

#define NT_SUCCESS(Status)	(((NTSTATUS)(Status)) >= 0)
#define MODULE_LAUNCH		"launch.xex"

void PrintDash();

void PrintDLVersion();

bool GetCPUKey();

void PrintCPUKey();

void SaveCPUKey(const char* file);