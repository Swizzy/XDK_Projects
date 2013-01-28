#include <xtl.h>
#include "AtgConsole.h"
#include "AtgUtil.h"
#include "AtgInput.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>
#include <sys/stat.h> 
#include "xfilecache.h"
#include <string>
#include "xbox.h"
#include <vector>
#include "mount.h"

using std::vector;
using std::string;
using std::ifstream;
using std::ofstream;
using std::fstream;

class NXE {
private:
	void readTitle() 
	{
		string fullPath = path + fileName;
		ifstream file;
		file.open ((char*)fullPath.c_str(), ifstream::in);
		if (file.is_open())
		{
			char buffer[_MAX_PATH];
			file.seekg(0x00000411);
			file.read(buffer,_MAX_PATH);
			swprintf_s(title, _MAX_PATH, L"%s", buffer);
			file.close();
		}
	}
public:
		string fileName;
		string path;
		wchar_t title[_MAX_PATH];
		NXE (string, string);
		bool status;	
};

string filePathzzz = "\\Content\\0000000000000000";
int fileCount = 0;
ATG::Console console;
bool debuglog = false;

extern "C" VOID XeCryptSha(LPVOID DataBuffer1, UINT DataSize1, LPVOID DataBuffer2, UINT DataSize2, LPVOID DataBuffer3, UINT DataSize3, LPVOID DigestBuffer, UINT DigestSize);

NXE::NXE (string strFileName, string strPath) {
	fileName = strFileName;
	path = strPath;
	readTitle();
	status = true;
}

vector<NXE> allNXE;
vector<string> devices;

int DeleteDirectory(const std::string &refcstrRootDirectory, bool bDeleteSubdirectories = true)
{
  bool            bSubdirectory = false;       // Flag, indicating whether
                                               // subdirectories have been found
  HANDLE          hFile;                       // Handle to directory
  std::string     strFilePath;                 // Filepath
  std::string     strPattern;                  // Pattern
  WIN32_FIND_DATA FileInformation;             // File information


  strPattern = refcstrRootDirectory + "\\*.*";
  hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
  if(hFile != INVALID_HANDLE_VALUE)
  {
    do
    {
      if(FileInformation.cFileName[0] != '.')
      {
        strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;

        if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
          if(bDeleteSubdirectories)
          {
            // Delete subdirectory
            int iRC = DeleteDirectory(strFilePath, bDeleteSubdirectories);
            if(iRC)
              return iRC;
          }
          else
            bSubdirectory = true;
        }
        else
        {
          // Set file attributes
          if(::SetFileAttributes(strFilePath.c_str(), FILE_ATTRIBUTE_NORMAL) == FALSE)
            return ::GetLastError();

          // Delete file
          if(::DeleteFile(strFilePath.c_str()) == FALSE)
			  return ::GetLastError();
        }
      }
    } while(::FindNextFile(hFile, &FileInformation) == TRUE);

    // Close handle
    ::FindClose(hFile);

    DWORD dwError = ::GetLastError();
    if(dwError != ERROR_NO_MORE_FILES)
      return dwError;
    else
    {
      if(!bSubdirectory)
      {
        // Set directory attributes
        if(::SetFileAttributes(refcstrRootDirectory.c_str(), FILE_ATTRIBUTE_NORMAL) == FALSE)
          return ::GetLastError();

        // Delete directory
        if(::RemoveDirectory(refcstrRootDirectory.c_str()) == FALSE)
          return ::GetLastError();
      }
    }
  }
  return 0;
}

bool FileExists(const char* strFilename) 
{
  struct stat tmp;
  if(stat(strFilename, &tmp) == 0)
    return true;
  return false;
}

void debugLog(char* output)
{
	if (!debuglog)
		return;
	FILE* fd;
	if (fopen_s(&fd, "game:\\debug.log", "ab") == 0)
	{
		fwrite(output, strlen(output), 1, fd);
		fprintf(fd, "\r\n");
		fclose(fd);
	}
}

void genlog()
{
	if (FileExists("game:\\debug.log"))
		return;
	FILE* fd;
	if (fopen_s(&fd, "game:\\debug.log", "w") == 0)
		fclose(fd);
}

bool MakeGOD(const char* file)
{
	char* SHA1Buffer[0xACBC];
	char* SHA1Digest[20];
	unsigned char* buffer;
	int size = 0;
	FILE* fd;
	if (fopen_s(&fd, file, "rb") == 0)
	{
		fseek(fd, 0, SEEK_END);
		size = ftell(fd);
		buffer = new unsigned char[size];
		fseek(fd, 0, SEEK_SET);
		if (fread(buffer, size, 1, fd) == 0)
		{
			fclose(fd);
			return false;
		}
		fclose(fd);
	}
	else
		return false;
	if (size < 0x413)
		return false;
	buffer[0x0] = 0x4C;
	buffer[0x1] = 0x49;
	buffer[0x2] = 0x56;
	buffer[0x3] = 0x45;
	int i = 0;
	for (i = 0x22C; i < 0x234; i++)
	{
		buffer[i] = 0xFF;
	}
	buffer[0x340] = 0x0;
	buffer[0x341] = 0x0;
	buffer[0x342] = 0xAD;
	buffer[0x343] = 0x0E;
	buffer[0x346] = 0x70;
	buffer[0x379] = 0x24;
	buffer[0x37A] = 0x5;
	buffer[0x37B] = 0x5;
	buffer[0x37C] = 0x11;
	for (i = 0x3FD; i < 0x412; i++)
	{
		buffer[i] = 0x0;
	}
	memcpy(SHA1Buffer, buffer + 0x344, 0xACBC);
	XeCryptSha(&SHA1Buffer, 0xACBC, NULL,0, NULL,0, &SHA1Digest, 20);
	memcpy(buffer + 0x32C, SHA1Digest, 20);
	if (fopen_s(&fd, file, "wb") == 0)
	{
		fwrite(buffer, size, 1, fd);
		fclose(fd);
		debugLog("New GOD container created, game should now launch without disk.");
		return true;
	}
	return false;
}

void nxetogod(NXE nxeGame)
{
	console.Format("Converting %ls to GOD... ", nxeGame.title);

	string goddirectory = nxeGame.path;
	goddirectory.replace(goddirectory.size() - 9, 9, "00007000");

	string olddatadirectory = nxeGame.path + nxeGame.fileName + ".data";

	string datadirectory = goddirectory + "\\" + nxeGame.fileName + ".data";

	string confile = nxeGame.path + nxeGame.fileName;

	string condirectory = nxeGame.path.substr(0, nxeGame.path.size() - 1);;

	//Create 00007000 directory if it doens't already exist
	::CreateDirectory(goddirectory.c_str(),0);

    //Append CON file name to existing path strings to give the full path
	goddirectory += "\\" + nxeGame.fileName;

	//Move CON file from 00004000 to 00007000
	if(FileExists(goddirectory.c_str()))
	{
		//Delete target file if it already exists
		::DeleteFile(goddirectory.c_str());
	}
	//Then move
	::MoveFile(confile.c_str(),goddirectory.c_str());

	//Move .data directory from 00004000 to 00007000
	if(::CreateDirectory(datadirectory.c_str(),0) == 0)
	{
		//Target directory exists in 00007000 - Delete it to empty it
		DeleteDirectory(datadirectory.c_str(),true);
		//Recreate it
		::CreateDirectory(datadirectory.c_str(),0);
	}
	//Now move data files there
	HANDLE hFind;
	WIN32_FIND_DATA wfd;

	string searchdir = olddatadirectory + "\\*";
	hFind = FindFirstFile( searchdir.c_str(), &wfd );	
	if(INVALID_HANDLE_VALUE != hFind)
	{
		do
		{		
			string oldFile = olddatadirectory + "\\" + wfd.cFileName;
			string newFile = datadirectory + "\\" + wfd.cFileName;
			::MoveFile(oldFile.c_str(), newFile.c_str());
		} while( FindNextFile( hFind, &wfd ));
		FindClose( hFind );
	}
	// Patch to GOD
	if (MakeGOD(goddirectory.c_str()) != true)
	{
		genlog();
		console.Format("FAILED\n");
		debugLog("Unable to open new Games On Demand Live file at:");
		debugLog((char*)goddirectory.c_str());
		nxeGame.status = false;
	}

	if(nxeGame.status)
	{
		console.Format("Success!\n");
		//Delete old data directory
		::RemoveDirectory(olddatadirectory.c_str());
		//Delete old 00004000 directory
		::RemoveDirectory(nxeGame.path.c_str());
	}
}

bool isNXE(WCHAR* path, string path2)
{
	if (!path2.compare(path2.length() - 9 , 8,"00004000") == 0)	
		return false;
	FILE* fd;
	if (_wfopen_s(&fd, path, L"rb") == 0)
	{
		unsigned char buf[4];
		unsigned char tmp[] = { 0, 0, 64, 0 };
		fseek(fd, 0x344, SEEK_SET);
		fread(buf, sizeof(buf), 1, fd);
		fclose(fd);
		if (memcmp(buf, tmp, sizeof(tmp)) == 0)		
			return true;		
	}
	return false;
}

HRESULT ScanDir(string strFind)
{
	HANDLE hFind;
	WIN32_FIND_DATA wfd;
	LPSTR lpPath = new char[MAX_PATH];
	LPWSTR lpPathW = new wchar_t[MAX_PATH];
	LPSTR lpFileName = new char[MAX_PATH];
	LPWSTR lpFileNameW = new wchar_t[MAX_PATH];
	string sFileName;
	strFind += "\\*";
	strFind._Copy_s(lpPath, strFind.length(),strFind.length());
	lpPath[strFind.length()]='\0';
	::MultiByteToWideChar( CP_ACP, NULL,lpPath, -1, lpPathW, MAX_PATH);
	hFind = FindFirstFile( lpPath, &wfd );
	int nIndex = 0;
	if(INVALID_HANDLE_VALUE == hFind)
	{
		debugLog("Invalid handle type - Directory most likely empty");
		debugLog(lpPath);
		debugLog(lpFileName);
	}
	else
	{
		nIndex = 0;
		do
		{
			sFileName = wfd.cFileName;
			sFileName._Copy_s(lpFileName, sFileName.length(), sFileName.length());
			lpFileName[sFileName.length()]='\0';
			::MultiByteToWideChar( CP_ACP, NULL,lpFileName, -1, lpFileNameW, MAX_PATH);
			if(FILE_ATTRIBUTE_DIRECTORY == wfd.dwFileAttributes)
			{
				string nextDir;
				nextDir += lpPath;
				string nextDir1;
				nextDir1 += lpFileName;
				nextDir.erase(nextDir.size() - 1, 1);
				nextDir += nextDir1;
				ScanDir(nextDir);
			}
			else
			{
				string filePath;
				string filePathX;
				string fileNameX;
				filePath += lpPath;
				string fileName;
				fileName += lpFileName;
				filePath.erase(filePath.size() - 1, 1);
				filePathX += filePath;
				fileNameX += fileName;
				filePath += fileName;
				LPSTR FileA = new char[MAX_PATH];
				LPSTR FileNameA = new char[MAX_PATH];
				LPSTR FilePathA = new char[MAX_PATH];
				filePathX._Copy_s(FilePathA, filePathX.length(),filePathX.length());
				fileNameX._Copy_s(FileNameA , fileNameX.length(),fileNameX.length());
				filePath._Copy_s(FileA, filePath.length(),filePath.length());
				FileA[filePath.length()]='\0';
				LPWSTR FileB = new wchar_t[MAX_PATH];
				::MultiByteToWideChar(CP_ACP, NULL, FileA, -1, FileB, MAX_PATH);
				if (isNXE(FileB,filePathX))
				{
					NXE temp(fileNameX,filePathX);
					allNXE.push_back(temp);
				}
			}
			nIndex++;
		}
		while(FindNextFile(hFind, &wfd));
		FindClose(hFind);
	}
	return S_OK;
}

void mountdrives()
{
	int mounted = 0;
	if (Map("HDD:","\\Device\\Harddisk0\\Partition1") == S_OK)
	{
		devices.push_back("HDD:");
		debugLog("hdd mounted");
		mounted++;
	}
	if (Map("USBMU0:", "\\Device\\Mass0PartitionFile\\Storage") == S_OK)
	{
		devices.push_back("USBMU0:");
		debugLog("usbmu0 mounted");
		mounted++;
	}
	if (Map("USBMU1:", "\\Device\\Mass1PartitionFile\\Storage") == S_OK)
	{
		devices.push_back("USBMU1:");
		debugLog("usbmu1 mounted");
		mounted++;
	}
	if (Map("USBMU2:", "\\Device\\Mass2PartitionFile\\Storage") == S_OK)
	{
		devices.push_back("USBMU2:");
		debugLog("usbmu2 mounted");
		mounted++;
	}
	if (Map("USBMU3:", "\\Device\\Mass3PartitionFile\\Storage") == S_OK)
	{
		devices.push_back("USBMU3:");
		debugLog("usbmu3 mounted");
		mounted++;
	}
	if (Map("USBMU4:", "\\Device\\Mass4PartitionFile\\Storage") == S_OK)
	{
		devices.push_back("USBMU4:");
		debugLog("usbmu4 mounted");
		mounted++;
	}
	if (mounted != 0)
	{
		debugLog("Drive(s) mounted, Scanning folders");
		console.Format("Scanning folders, please wait...");
	}
	else
	{
		debugLog("No Drives found!");
		console.Format("No drives found!\n");
	}
}

//--------------------------------------------------------------------------------------
// Name: main
// Desc: Entry point to the program
//--------------------------------------------------------------------------------------
VOID __cdecl main()
{
	bool keypush = false;
	console.Create("embed:\\font", 0x00000000, 0xFFFF6600);
	console.Format( "NXE2GOD V1.2 by Swizzy original source by Dstruktiv\n" );
	//genlog(); //<--- only use this when you're lazy ;)
	debuglog = FileExists("game:\\debug.log");
	mountdrives();
	console.Format("\n");
	for(unsigned int i = 0; i < devices.size(); i++)
	{
		if (devices.size() == 0)
			break;
		unsigned int tmpsize = allNXE.size();
		console.Format("Scanning %s\\ for NXE titles...", devices[i].c_str());
		string tmp = devices[i] + filePathzzz;
		ScanDir(tmp);
		if (allNXE.size() == tmpsize)
			console.Format("No NXE titles found!\n");
		else if (tmpsize != 0)
			console.Format("Found %d NXE titles!\n", allNXE.size() - tmpsize);
		else
			console.Format("Found %d NXE titles!\n", allNXE.size());
	}
	if ((allNXE.size() == 0) && (devices.size() == 0))	
		console.Format("\nNo NXE titles found\n\nPush any key to exit" );	
	else if (allNXE.size() == 0)
		console.Format("\n\nPush any key to exit" );
	else
	{
		console.Format("\nNXE Files found:\n\n");
		for(unsigned int i = 0; i < allNXE.size(); i++)
		{
			console.Format("%ls at location: %s%s\n", allNXE[i].title, allNXE[i].path.c_str(), allNXE[i].fileName.c_str());
		}
		console.Format("\nPush A to convert files from NXE to GOD or B to cancel and quit to NXE\n\n");
		while(!keypush)
		{
			ATG::GAMEPAD* pGamepad = ATG::Input::GetMergedInput();
			if( pGamepad->wPressedButtons & XINPUT_GAMEPAD_A)
				keypush = true;
			if( pGamepad->wPressedButtons & XINPUT_GAMEPAD_B)
				XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP, 0);
		}
		console.Format("Converting NXE2GOD, please wait...\n\n");
		for(unsigned int i = 0; i < allNXE.size(); i++) 
			nxetogod(allNXE[i]);
		console.Format( "\nProcessing complete, push any key to exit" );
	}
	keypush = false;
	while(!keypush)
	{
		ATG::GAMEPAD* pGamepad = ATG::Input::GetMergedInput();
		if( pGamepad->wPressedButtons)		
			XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP, 0);
	}
}