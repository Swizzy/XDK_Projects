#include "mount.h"
#include <stdio.h>

HRESULT Map(const char* szDrive, char* szDevice)
{
    STRING DeviceName, LinkName;
	CHAR szDestinationDrive[MAX_PATH];
	sprintf_s(szDestinationDrive, MAX_PATH, "\\??\\%s", szDrive);
	RtlInitAnsiString(&DeviceName, szDevice);
	RtlInitAnsiString(&LinkName, szDestinationDrive);
	ObDeleteSymbolicLink(&LinkName);
	return (HRESULT)ObCreateSymbolicLink(&LinkName, &DeviceName);
}

HRESULT unMap(const char* szDrive)
{
	STRING LinkName;
	CHAR szDestinationDrive[MAX_PATH];
	sprintf_s(szDestinationDrive, MAX_PATH, "\\??\\%s", szDrive);
	RtlInitAnsiString(&LinkName, szDestinationDrive);
	return ObDeleteSymbolicLink(&LinkName);
}