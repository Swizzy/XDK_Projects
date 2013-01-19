#ifndef MOUNT_H
#define MOUNT_H
#include <xtl.h>

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} STRING, *PSTRING;

#ifdef __cplusplus
extern "C" {
#endif
	VOID RtlInitAnsiString(PSTRING DestinationString, PCHAR SourceString);
	HRESULT ObDeleteSymbolicLink(PSTRING SymbolicLinkName);
	HRESULT ObCreateSymbolicLink(PSTRING SymbolicLinkName, PSTRING DeviceName);
#ifdef __cplusplus
}
#endif
HRESULT Map(const char* szDrive, char* szDevice);
HRESULT unMap(const char* szDrive);
#endif