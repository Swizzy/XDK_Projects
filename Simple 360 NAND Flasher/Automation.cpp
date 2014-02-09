#include "Automation.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

extern "C" void __cdecl dprintf( const char* strFormat, ... );
extern "C" void HalReturnToFirmware(unsigned long mode);

unsigned int calc_crc32(unsigned int& crc, const void* buf, size_t size)
{
	const unsigned char* p;
	p = (unsigned char*)buf;
	crc = crc ^ ~0U;
	while (size--)
		crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
	return crc ^ ~0U;
}

void PrintValue(unsigned int hash, unsigned int hash2)
{
	dprintf("\nCalculated value: %08X", hash);
	dprintf("\nExpected value:   %08X", hash2);
	dprintf("\n");
}

bool fexists(const char *filename)
{
	struct stat info;
	if (stat(filename, &info) == 0)
		return true;
	return false;
}

bool CheckCRC32(string file, string hashfile)
{
	FILE* f;
	unsigned int crc = 0;
	unsigned char buf[4096];
	dprintf(" * Checking %s Crc32: ", file.c_str());
	fopen_s(&f, file.c_str(), "rb");
	if (f != NULL)
	{
		while (fread(&buf, 1, sizeof(buf), f) == sizeof(buf))		
			crc = calc_crc32(crc, buf, sizeof(buf));
		fclose(f);
	}
	else
	{
		dprintf("\n ! ERROR Opening %s for reading! Aborting!", file.c_str());
		return false;
	}
	fopen_s(&f, hashfile.c_str(), "r");
	if (f != NULL)
	{
		unsigned int crc2 = 0;
		fscanf_s(f, "%X", &crc2);
		PrintValue(crc, crc2);
		return (crc == crc2);			
	}
	else
	{
		dprintf("\n ! ERROR Opening %s for reading! Aborting!", hashfile.c_str());
	}
	return false;
}

void GenerateCRC32(string file)
{
	FILE* f;
	unsigned int crc = 0;
	unsigned char buf[4096];
	dprintf("Calculating CRC32 for: %s\n", file.c_str());
	fopen_s(&f, file.c_str(), "rb");
	if (f != NULL)
	{
		while (fread(&buf, 1, sizeof(buf), f) == sizeof(buf))		
			crc = calc_crc32(crc, buf, sizeof(buf));		
		fclose(f);
		dprintf("Calculated CRC32 value: %08X\n", crc);
		string hashfile;
		unsigned found = file.find_last_of(".");
		if (found != string::npos)
			hashfile = file.substr(0, found + 1) + "crc32";
		else
			hashfile = file + ".crc32";
		dprintf(" * Attempting to save the CRC32 value to %s...\n", hashfile.c_str());
		fopen_s(&f, hashfile.c_str(), "w");
		if (f != NULL)
		{
			fprintf(f, "%08X", crc);
			fclose(f);
			dprintf(" * Sucessfully saved the CRC32 value!\n");
		}
		else
			dprintf(" ! ERROR: Unable to open %s for write\n", hashfile.c_str());
	}
	else
		dprintf("\n ! ERROR Opening %s for reading! Aborting!", file.c_str());
}

//bool CheckSHA1(string file, string hashfile)
//{
//	FILE* f;
//	unsigned int sha = 0;
//	if (fopen_s(&f, hashfile.c_str(), "r"))
//	{
//		unsigned int sha2 = 0;
//		fscanf_s(f, "%X", &sha2);
//		PrintValue(sha, sha2);
//		return (sha == sha2);			
//	}
//	return false;
//}

bool CheckHash(string file)
{
	unsigned found = file.find_last_of(".");
	string basename;
	if (found != string::npos)
		basename = file.substr(0, found + 1);
	else
		basename = file;
	if (fexists(file.c_str()))
	{
		string tmp = basename + "crc32";
		dprintf(" * Looking for %s... ", tmp.c_str());		
		if (fexists(tmp.c_str()))
		{
			dprintf("Found!\n * Checking CRC32 Hash...\n");
			return CheckCRC32(file, tmp);
		}
		dprintf("NOT Found!\n");
		/*tmp = basename + "sha1";
		dprintf("* Looking for %s... ", tmp.c_str());		
		if (fexists(tmp.c_str()))
		{
			dprintf("Found! Checking SHA1 Hash...\n");
			return CheckSHA1(file, tmp);
		}
		dprintf("NOT Found!\n");*/
	}
	return false;
}

void GenerateHash(const char* filename)
{
	GenerateCRC32(string(filename));
}

int CheckMode(const char* filename)
{
	FILE* fd;
	fopen_s(&fd, filename, "r");
	if (fd != NULL)
	{
		char cmd[512];
		fgets(cmd, 512, fd);
		fclose(fd);
		int i;
		for (i = 0; cmd[i]; i++)
			cmd[i] = (char)tolower(cmd[i]);
		remove(filename);
		if (strncmp(cmd, "dump", 4) == 0)
			return 1;
		if (strncmp(cmd, "exit", 4) == 0)
			return 4;
		if (strncmp(cmd, "safe", 4) == 0)
			return 3;
		if (strncmp(cmd, "read", 4) == 0)
			return 1;
		if (strncmp(cmd, "write", 5) == 0)
			return 2;
		if (strncmp(cmd, "flash", 5) == 0)
			return 2;
		if (strncmp(cmd, "reboot", 6) == 0)
			return 5;
	}
	else
		dprintf(" ! ERROR: Couldn't open %s for read!\n", filename);
	return -1;
}