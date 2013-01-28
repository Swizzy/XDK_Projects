#include "OutputConsole.h"
#include "AtgConsole.h"
#include "AtgInput.h"
#include "AtgUtil.h"
#include <time.h>
#include "Corona4G.h"
#include "Automation.h"
#include "keyextract.h"

extern "C" {
#include "xenon_sfcx.h"
}

ATG::GAMEPAD*	m_pGamepad; // Gamepad for input
time_t start,end; //Timer times for measuring time difference
double tdiff; // Double for time difference
bool started = false, dumped = false, dump_in_progress = false, MMC = false, write = false, AutoMode = false;
unsigned int config = 0;

extern "C" VOID HalReturnToFirmware(DWORD mode); // To Shutdown console when done ;)
extern "C" VOID XInputdPowerDownDevice(DWORD flag); // To Kill controllers
static unsigned long bswap32(unsigned long t) { return ((t & 0xFF) << 24) | ((t & 0xFF00) << 8) | ((t & 0xFF0000) >> 8) | ((t & 0xFF000000) >> 24); } //For ECC calculation
VOID KillControllers()
{
	XInputdPowerDownDevice(0x10000000);
	XInputdPowerDownDevice(0x10000001);
	XInputdPowerDownDevice(0x10000002);
	XInputdPowerDownDevice(0x10000003);
}

bool CheckPage(BYTE* page)
{
	unsigned int* data = (unsigned int*)page;
	unsigned int i=0, val=0, v=0;
	unsigned char edc[4];	
	for (i = 0; i < 0x1066; i++)
	{
		if (!(i & 31))
			v = ~bswap32(*data++);		
		val ^= v & 1;
		v>>=1;
		if (val & 1)
			val ^= 0x6954559;
		val >>= 1;
	}

	val = ~val;
	// 26 bit ecc data
	edc[0] = ((val << 6) | (page[0x20C] & 0x3F)) & 0xFF;
	edc[1] = (val >> 2) & 0xFF;
	edc[2] = (val >> 10) & 0xFF;
	edc[3] = (val >> 18) & 0xFF;
	return ((edc[0] == page[0x20C]) && (edc[1] == page[0x20D]) && (edc[2] == page[0x20E]) && (edc[3] == page[0x20F]));
}

int HasSpare(char* filename)
{
	BYTE buf[0x630];
	FILE* fd;
	dprintf("Checking %s for spare data... ", filename);
	if (fopen_s(&fd, filename, "rb") != 0)
	{		
		dprintf("\nERROR: Unable to open %s for reading!\n", filename);
		return -1;
	}
	if (fread_s(buf, 0x630, 0x630, 1, fd) != 1)
	{
		dprintf("\nERROR: Unable to read 0x630 bytes from %s!\n", filename);
		fclose(fd);
		return -1;
	}
	fclose(fd);
	if (buf[0] != 0xFF && buf[1] != 0x4F)
	{
		dprintf("\nERROR: %s Doesn't contain the magic bytes expected for an Xbox 360 NAND image, aborting!\n", filename);
		return -1;
	}	
	for (int offset = 0; offset < 0x630; offset += 0x210)
	{
		if (CheckPage(&buf[offset]))
		{
			dprintf("Spare data detected!\n", filename);
			return 0;
		}
	}
	dprintf("No spare data detected!\n", filename);
	return 1;
}

void AutoCountdown(int timeout = 5)
{
	for (; timeout > 0; timeout--)
	{
		dprintf("\rYou have %d Seconds to power off your console before the code continues...", timeout);
		Sleep(1000);
	}
	dprintf("Time is up! Hope you didn't make a misstake! ;)\n");
}

VOID flasher()
{
	if (!AutoMode)
	{
		dprintf("Press Start to flash your nand or press anything else to exit!\n\n");
		for(;;)
		{
			m_pGamepad = ATG::Input::GetMergedInput();
			if (m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_START)
				break;
			else if (m_pGamepad->wPressedButtons)
				XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP, 0);
		}
	}
	started = true;
	KillControllers();
	ClearConsole();
	time(&start);
	dprintf("WARNING! DO NOT TOUCH YOUR CONSOLE OR CONTROLLER DURING THE FLASH PROCESS!!\nThe console will reboot when it's done!\n\n");
	int tmp = HasSpare("game:\\updflash.bin");
	if (tmp == -1)
		XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP, 0);
	bool isEcced = (tmp == 0);	
	if (!MMC)
	{
		if (!isEcced)
		{
			if (!AutoMode)
			{
				dprintf("\n\nWARNING! You are about to flash an image that don't contain SPARE data to a machine that requires it!\nIf you know what you are doing Press Start to continue or anything else to exit!\n");
				for(;;)
				{
					m_pGamepad = ATG::Input::GetMergedInput();
					if (m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_START)
						break;
					else if (m_pGamepad->wPressedButtons)
						XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP, 0);				
				}
			}
			else
			{
				dprintf("\n\nWARNING! You are about to flash an image that don't contain SPARE data to a machine that requires it!\nPower off your console to abort!");
				AutoCountdown();
			}
		}
		unsigned int r = sfcx_init();
		sfcx_printinfo(r);
		dprintf("\n\n");
		try_rawflash("game:\\updflash.bin");
		sfcx_setconf(config);
	}
	else
	{
		if (isEcced)
		{
			if (!AutoMode)
			{
				dprintf("\n\nWARNING! You are about to flash an image that contains SPARE data to a machine that don't require it!\nIf you know what you are doing Press Start to continue or anything else to exit!\n");
				for(;;)
				{
					m_pGamepad = ATG::Input::GetMergedInput();
					if (m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_START)
						break;
					else if (m_pGamepad->wPressedButtons)
						XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP, 0);
				}
			}
			else
			{
				dprintf("\n\nWARNING! You are about to flash an image that contains SPARE data to a machine that don't require it!\nPower off your console to abort!");
				AutoCountdown();
			}
		}
		try_rawflash4g("game:\\updflash.bin");
	}
	time(&end);
	tdiff = difftime(end,start);
	dprintf("Completed after %4.2f seconds\n", tdiff);
	dprintf("Rebooting in ");
	for (int i = 5; i > 0; i--)
	{
		dprintf("%i", i);
		for (int j = 0; j < 4; j++)
		{
			Sleep(250);
			dprintf(".");
		}
	}
	dprintf("BYE!");
	HalReturnToFirmware(2);
}

VOID dumper(char *filename)
{
	if (!AutoMode)
	{
		if (fexists(filename))
		{
			dprintf(" **** WARNING: %s already exists! ****\nPress Start if you want to overwrite this file now!", filename);
			for(;;)
			{
				m_pGamepad = ATG::Input::GetMergedInput();
				if (m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_START)
					break;
				else if (m_pGamepad->wPressedButtons)
					XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP, 0);
			}
		}
	}
	started = true;
	if (!MMC)
	{
		sfcx_init();
		sfcx_setconf(config);
		int size = sfc.size_bytes_phys;
		if((size == (RAW_NAND_64*4)) || (size == (RAW_NAND_64*8)))
		{
			if (!AutoMode)
			{
				dprintf("Press A to Dump System Partition only (64MB, Recommended!)\n");
				dprintf("Press B to Dump Full NAND (256MB/512MB, this may take a while...)\n");
				dprintf("Press Back to abort dump process...\n");
				for(;;)
				{
					m_pGamepad = ATG::Input::GetMergedInput();
					if (m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_A)
					{
						size = RAW_NAND_64;
						break;
					}
					else if (m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_B)
						break;
					else if (m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_BACK)
						XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP, 0);
					else if (m_pGamepad->wPressedButtons)
					{
						dprintf("Try again:\n");
						dprintf("Press A to Dump System Partition only (64MB, Recommended!)\n");
						dprintf("Press B to Dump Full NAND (256MB/512MB, this may take a while...)\n");
						dprintf("Press Back to abort dump process...\n");
					}
				}
			}
			else
			{
				if (size == (RAW_NAND_64*4))
					size = 256;
				else
					size = 512;
				dprintf(" * %d MB NAND Detected! Setting dump size to 64MB!", size);
				size = RAW_NAND_64;
			}
		}
		ClearConsole();
		time(&start);
		unsigned int r = sfcx_init();
		sfcx_printinfo(r);
		try_rawdump(filename, size);
		sfcx_setconf(config);
	}
	else
	{
		ClearConsole();
		time(&start);
		try_rawdump4g(filename);
	}
	time(&end);
	tdiff = difftime(end,start);
	dprintf("Completed after %4.2f seconds\n", tdiff);
	dumped = true;
}

void PrintExecutingCountdown(int max)
{
	for (; max > 0; max--)
	{
		dprintf("\rExecuting command in %d seconds", max);
		Sleep(1000);
	}
	dprintf("\rExecuting command!\n");
}

void TryAutoMode()
{
	dprintf(" * Looking for simpleflasher.cmd for automatic features...\n");
	if (fexists("game:\\simpleflasher.cmd"))
	{
		AutoMode = true;
		dprintf("game:\\simpleflasher.cmd Found!\n * Entering Automatic mode!\n");
		int mode = CheckMode("game:\\simpleflasher.cmd");
		if (mode == 1) //AutoDump
		{
			dprintf(" * AutoDump command found!\n * Executing command!\n\n");
			dumper("game:\\flashdmp.bin");
			GenerateHash("game:\\flashdmp.bin");
		}
		else if (mode == 2) //AutoFlash
		{
			dprintf(" * AutoFlash command found!\n\n");
			if (CheckHash("game:\\updflash.bin"))
			{
				PrintExecutingCountdown(5);
				flasher();
			}
			else
				dprintf(" ! ERROR: Hash don't match, or file don't exist... Aborting!\n");
		}
		else if (mode == 3) //AutoSafeFlash
		{
			dprintf(" * AutoSafeFlash command found!\n\n");
			if (CheckHash("game:\\updflash.bin"))
			{
				PrintExecutingCountdown(5);
				dumper("game:\\recovery.bin");
				AutoCountdown();
				flasher();
			}
			else
				dprintf(" ! ERROR: Hash don't match, or file don't exist... Aborting!\n");
		}
		else if (mode == 4) //AutoExit, only want key...
		{
			dprintf(" * AutoExit command found!\n\n");
			PrintExecutingCountdown(5);
		}
		else if (mode == 5) //AutoReboot Hard Reset
		{
			dprintf(" * AutoReboot command found!\n\n");
			PrintExecutingCountdown(5);
			HalReturnToFirmware(2);
		}
		else
		{
			dprintf("! ERROR: Bad command file please read the readme!\nReturning to manual mode!\n");
			AutoMode = false;
			return;
		}
		if (AutoMode)
			XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP, 0); //We don't want to return to manual mode ;)
	}
	else
	{
		dprintf("game:\\simpleflasher.cmd NOT Found!\n * Entering Manual mode!\n\n");
	}
}

void PrintConsoleInfo(bool GotKey)
{
	dprintf("\n============= Console information: =============\n\n");
	PrintDash();
	if (GotKey)
		PrintCPUKey();
	dprintf("\n================================================\n\n");
}

//--------------------------------------------------------------------------------------
// Name: main()
// Desc: Entry point to the program
//--------------------------------------------------------------------------------------
VOID __cdecl main()
{
	// Initialize the console window
	write = fexists("game:\\updflash.bin");
	MakeConsole("embed:\\font", CONSOLE_COLOR_BLACK, CONSOLE_COLOR_GOLD);

	dprintf("Simple 360 NAND Flasher by Swizzy v1.3 (BETA)\n\n");

	dprintf(" * Detecting NAND Type...\n");
	MMC = (sfcx_detecttype() == 1); // 1 = MMC, 0 = RAW NAND
	if (!MMC)
		config = sfcx_getconf();
	bool GotKey = false;
	dprintf(" * Attempting to grab CPUKey...\n");
	if (GetCPUKey())
	{
		SaveCPUKey("game:\\cpukey.txt");
		GotKey = true;
	}
	else
	{
		dprintf(" ! ERROR: Your dashboard is to old for this feature, sorry... use xell!\n");
		GotKey = false;
	}
	PrintConsoleInfo(GotKey);
	TryAutoMode();
	if (write)
	{
		if (!MMC)
		{
			dprintf("Press A if you want to flash your nand with Rawflash v4\n");
			dprintf("Press B if you want to safeflash your nand with Rawflash v4 (Dump + Write)\n");
		}
		else
		{
			dprintf("Press A if you want to flash your nand with Rawflash4G v1\n");
			dprintf("Press B if you want to safeflash your nand with Rawflash4G v1 (Dump + Write)\n");
		}
	}
	if (!MMC)
		dprintf("Press X if you want to dump your nand with Rawdump v1\n");
	else
		dprintf("Press X if you want to dump your nand with Rawdump4G v1\n");
	dprintf("If you press anything else the application will close...\n");
	for(;;)
	{
		m_pGamepad = ATG::Input::GetMergedInput();
		if (!started)
		{
			if ((m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_A) && (write))
			{
				flasher();
			}
			else if ((m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_B) && (!dumped) && (write))
			{
				dumper("game:\\recovery.bin");
				flasher();
			}
			else if ((m_pGamepad->wPressedButtons & XINPUT_GAMEPAD_X) && (!dumped))
			{
				dumper("game:\\flashdmp.bin");
				dprintf("Press any button to exit!\n");
			}
			else if (m_pGamepad->wPressedButtons) { break; }
		}
		else if ((m_pGamepad->wPressedButtons) && (dumped)) { break; }
	}
	if (!MMC)
		sfcx_setconf(config);
}