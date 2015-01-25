#ifndef __Corona4G_h
#define __Corona4G_h

#include <stdio.h>
#include <xtl.h>

typedef LONG NTSTATUS;

typedef struct _STRING 
{
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} STRING, *PSTRING;

typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID Pointer;
	} st;
	ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef struct _OBJECT_ATTRIBUTES {
	HANDLE RootDirectory;
	PSTRING ObjectName;
	ULONG Attributes;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef VOID (NTAPI *PIO_APC_ROUTINE) (
	IN PVOID ApcContext,
	IN PIO_STATUS_BLOCK IoStatusBlock,
	IN ULONG Reserved
	);

#ifdef __cplusplus
extern "C" {
#endif
	VOID RtlInitAnsiString(PSTRING DestinationString, PCHAR SourceString);	
	HRESULT ObDeleteSymbolicLink(PSTRING SymbolicLinkName);
	HRESULT ObCreateSymbolicLink(PSTRING SymbolicLinkName, PSTRING DeviceName);
	NTSTATUS NtOpenFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, DWORD ShareAccess, DWORD OpenOptions);
	NTSTATUS NtClose(HANDLE Handle);
	NTSTATUS NtWriteFile(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer, DWORD Length, PLARGE_INTEGER ByteOffset);

	HRESULT mount(const char* szDrive, char* szDevice);
	void try_rawdump4g(char* filename);
	void try_rawflash4g(char* filename);
	int getflashsz(void);
	PBYTE FileToBuffer(char * file, PDWORD filesize);
	BOOL Corona4GWrite(PBYTE data);
	VOID KillControllers();

#ifdef __cplusplus
}
#endif

#define FILE_SYNCHRONOUS_IO_NONALERT	0x20

#endif