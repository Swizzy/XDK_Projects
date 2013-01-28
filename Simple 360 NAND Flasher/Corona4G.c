#include <xtl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "Corona4G.h"
#include "OutputConsole.h"

#define BUF_SIZE	0x8000
DWORD flash_sz = 0, TTL_REPS = 0, totalBytes = 0, totalRead = 0, totalWrote = 0;
BYTE buf[BUF_SIZE];

HRESULT mount(const char* szDrive, char* szDevice)
{
	STRING DeviceName, LinkName;
	CHAR szDestinationDrive[MAX_PATH];
	sprintf_s(szDestinationDrive, MAX_PATH, "\\??\\%s", szDrive);
	RtlInitAnsiString(&DeviceName, szDevice);
	RtlInitAnsiString(&LinkName, szDestinationDrive);
	ObDeleteSymbolicLink(&LinkName);
	return (HRESULT)ObCreateSymbolicLink(&LinkName, &DeviceName);
}

char* GetSizeReadable(unsigned int i)
{
	char * ret = "";
	if (i >= 0x40000000) // Gigabyte
		sprintf_s(ret, 512, "%3.2f GB", (float)(i >> 20) / (float)1024);
	else if (i >= 0x100000) // Megabyte
		sprintf_s(ret, 512, "%3.2f MB", (float)(i >> 10) / (float)1024);
	else if (i >= 0x400) // Kilobyte
		sprintf_s(ret, 512, "%3.2f KB", (float)i / (float)1024);
	else
		sprintf_s(ret, 512, "%3.2f B", (float)i / (float)1024);
	return ret;
}

int getflashsz(void)
{
	FILE * fd;
	dprintf("Getting NAND Size...\n");
	if (mount("Flash:", "\\Device\\Flash") != 0)
	{
		dprintf("ERROR: Unable to mount Flash!");
		return 0;
	}
	if (fopen_s(&fd, "Flash:", "rb") != 0)
	{
		dprintf("ERROR: Unable to open Flash: for read\n");
		fclose(fd);
		return 0;
	}
	fseek(fd, 0L, SEEK_END);
	flash_sz = ftell(fd);
	fclose(fd);
	dprintf(" * Size: %x (%s)\n", flash_sz, GetSizeReadable(flash_sz));
	return 1;
}

void try_rawdump4g(char* filename)
{
	HANDLE flash, outf;
	DWORD bRead, bWrote;
	DWORD i;
	totalRead = 0;
	totalWrote = 0;
	dprintf(" * rawdump4g v1 started (by Swizzy)\n");
	if (getflashsz() == 0)
	{
		dprintf("ERROR: Unable to get size of flash!\n");
		return;
	}
	TTL_REPS = (flash_sz/BUF_SIZE);
	flash = CreateFile("Flash:", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);	
	if(flash == INVALID_HANDLE_VALUE)
	{
		dprintf("ERROR: Unable to open Flash for reading\n");
		return;
	}
	outf = CreateFile(filename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(outf == INVALID_HANDLE_VALUE)
	{
		dprintf("ERROR: Unable to open %s for writing\n", filename);
		return;
	}
		dprintf("%s opened OK, attempting to read 0x%x bytes (%s) from flash...\n", filename, flash_sz, GetSizeReadable(flash_sz));
		for(i = 0; i < TTL_REPS; i++)
		{
			ReadFile(flash, buf, BUF_SIZE, &bRead, NULL);
			if(bRead != BUF_SIZE)
				dprintf("\rerror reading 0x%x bytes at offset 0x%x (read: 0x%x bytes)!\n", BUF_SIZE, totalBytes, bRead);
			WriteFile(outf, buf, BUF_SIZE, &bWrote, NULL);
			if(bWrote != BUF_SIZE)
				dprintf("\rerror writing 0x%x bytes at offset 0x%x (wrote: 0x%x bytes)!\n", BUF_SIZE, totalBytes, bWrote);
			totalBytes += BUF_SIZE;
			totalRead += bRead;
			totalWrote += bWrote;
			if(i%0x20) // only update this every 32 buffer fills to speed things along...
				dprintf("\rProcessed 0x%08x of 0x%08x bytes (%3.2f %%)", totalBytes, flash_sz, (((float)totalBytes / (float)flash_sz) * 100));
		}
		dprintf("\r");
		CloseHandle(outf);
		CloseHandle(flash);
		if (totalRead - totalWrote == 0)
			dprintf("\nDone! successfully dumped 0x%x bytes to %s\n", totalBytes, filename);
		else
			dprintf("\nDone! read 0x%x bytes from NAND, wrote 0x%x bytes to %s\n", totalRead, totalWrote, filename);
}

void try_rawflash4g(char* filename)
{
	PBYTE data;
	DWORD filesize;
	dprintf(" * rawflash4g v1 started (by Swizzy)\n");	
	if (getflashsz() == 0)
	{
		dprintf("ERROR: Unable to get size of flash!\n");
		return;
	}
	dprintf(" * Reading in Image to Memory...\n");
	data = FileToBuffer(filename, &filesize);
	if (data == NULL)
		return;
	if (flash_sz < filesize)
	{
		dprintf("ERROR: %s is bigger then Flash! Aborting!\n", filename);
		free(data);
		return;
	}
	if (flash_sz > filesize)
	{
		dprintf("ERROR: %s is smaller then Flash! Aborting!\n", filename);
		free(data);
		return;
	}
	dprintf(" * Writing image to Flash...\n");
	if (Corona4GWrite(data))
		dprintf("Successfully wrote 0x%X bytes from %s to Flash!\n", filesize, filename);
	else
	dprintf("ERROR: Failed to write 0x%X bytes from %s to Flash!\n", filesize, filename);
}

BOOL Corona4GWrite(PBYTE data)
{
	HANDLE hFile;
	OBJECT_ATTRIBUTES atFlash;
	IO_STATUS_BLOCK ioFlash;
	DWORD sta = 0, bWrote = 0, len = flash_sz;
	PBYTE srcdata = data;
	LARGE_INTEGER lOffset;
	STRING nFlash;
	BOOL error;
	RtlInitAnsiString(&nFlash, "\\Device\\Flash");
	atFlash.RootDirectory = 0;
	atFlash.ObjectName = &nFlash;
	atFlash.Attributes = FILE_ATTRIBUTE_DEVICE;
	if (NtOpenFile(&hFile, GENERIC_WRITE|GENERIC_READ|SYNCHRONIZE, &atFlash, &ioFlash, OPEN_EXISTING, FILE_SYNCHRONOUS_IO_NONALERT) != 0)
	{
		dprintf("ERROR: Unable to open Flash for write! Aborting!\n");
		return FALSE;
	}
	lOffset.QuadPart = 0;
	error = FALSE;
	while (len > 0x4000)
	{
		KillControllers();
		bWrote = 0;
		sta = NtWriteFile(hFile, 0, 0, 0, &ioFlash, srcdata, 0x4000, &lOffset);
		if (sta < 0)
		{
			dprintf("\rERROR: Unable to write 0x%X bytes to flash, ERROR: 0x%X Offset: 0x%X", 0x4000, GetLastError(), lOffset.LowPart);		
			error = TRUE;
		}
		else
			bWrote = 0x4000;
		len -= bWrote;
		srcdata += bWrote;
		lOffset.QuadPart += bWrote;
		if ((lOffset.LowPart%(0x4000*32)) == 0)
			dprintf("\rProcessed 0x%X bytes of 0x%X (%3.2f %%)", lOffset.LowPart, flash_sz, (((float)lOffset.LowPart / (float)flash_sz) * 100));
	}
	dprintf("\r");
	if (len > 0)
	{
		bWrote = 0;
		sta = NtWriteFile(hFile, 0, 0, 0, &ioFlash, srcdata, len, &lOffset);
		if (sta < 0)
		{
			dprintf("ERROR: Unable to write 0x%X bytes to flash, ERROR: 0x%X Offset: 0x%X", len, GetLastError(), lOffset.LowPart);		
			error = TRUE;
		}
		else
			bWrote = len;
		lOffset.QuadPart += bWrote;
	}
	NtClose(hFile);
	if (error)
		return FALSE;
	return TRUE;
}

PBYTE FileToBuffer(char * file, PDWORD filesize)
{
	HANDLE inf;
	PBYTE dest = NULL;
	DWORD bRead, lSize;
	filesize[0] = 0;
	inf = CreateFile(file, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (inf == INVALID_HANDLE_VALUE)
	{
		dprintf("ERROR: Unable to open %s for reading, error: 0x%X\n", file, GetLastError());
		return dest;
	}
	lSize = GetFileSize(inf, NULL);
	dest = (PBYTE)malloc(lSize);
	if (dest == NULL)
	{
		dprintf("ERROR: Unable to allocate 0x%X bytes buffer for %s\n", lSize, file);
		CloseHandle(inf);
		return dest;
	}
	ReadFile(inf, dest, lSize, &bRead, NULL);
	if (bRead != lSize)
	{
		dprintf("ERROR: Unable to read 0x%X bytes from %s (read: 0x%X bytes)\n", lSize, file, bRead);
		CloseHandle(inf);
		return NULL;
	}
	CloseHandle(inf);
	filesize[0] = lSize;
	return dest;
}