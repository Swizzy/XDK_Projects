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
	dprintf(MSG_CHECKING_FOR_SPARE, filename);
	if (fopen_s(&fd, filename, "rb") != 0)
	{		
		dprintf(MSG_ERROR MSG_UNABLE_TO_OPEN_FOR_READING, filename);
		return -1;
	}
	if (fread_s(buf, 0x630, 0x630, 1, fd) != 1)
	{
		dprintf(MSG_ERROR MSG_UNABLE_TO_READ_0X630_BYTES_FROM, filename);
		fclose(fd);
		return -1;
	}
	fclose(fd);
	if (buf[0] != 0xFF && buf[1] != 0x4F)
	{
		dprintf(MSG_ERROR MSG_BAD_MAGIC, filename);
		return -1;
	}	
	for (int offset = 0; offset < 0x630; offset += 0x210)
	{
		if (CheckPage(&buf[offset]))
		{
			dprintf(MSG_SPARE_DETECTED, filename);
			return 0;
		}
	}
	dprintf(MSG_SPARE_NOT_DETECTED, filename);
	return 1;
}

void AutoCountdown(int timeout = 5)
{
	for (; timeout > 0; timeout--)
	{
		dprintf(MSG_YOU_HAVE_SECONDS_BEFORE_CONTINUE, timeout);
		Sleep(1000);
	}
	dprintf(MSG_TIMES_UP);
}

VOID flasher()
{
	if (!AutoMode)
	{
		dprintf(MSG_PRESS_START_TO_FLASH);
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
	dprintf(MSG_WARNING_DO_NOT_TOUCH_CONSOLE_OR_CONTROLLER);
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
				dprintf(MSG_WARNING_YOU_ARE_ABOUT_TO_FLASH_NO_SPARE_TO_SPARE_CONSOLE);
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
				dprintf(MSG_WARNING_YOU_ARE_ABOUT_TO_FLASH_NO_SPARE_TO_SPARE_CONSOLE_AUTO);
				AutoCountdown();
			}
		}
		unsigned int r = sfcx_init();
		sfcx_printinfo(r);
#ifdef USE_UNICODE
		dprintf(L"\n\n");
#else
		dprintf("\n\n");
#endif
		try_rawflash("game:\\updflash.bin");
		sfcx_setconf(config);
	}
	else
	{
		if (isEcced)
		{
			if (!AutoMode)
			{
				dprintf(MSG_WARNING_YOU_ARE_ABOUT_TO_FLASH_SPARE_TO_NO_SPARE_CONSOLE);
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
				dprintf(MSG_WARNING_YOU_ARE_ABOUT_TO_FLASH_SPARE_TO_NO_SPARE_CONSOLE_AUTO);
				AutoCountdown();
			}
		}
		try_rawflash4g("game:\\updflash.bin");
	}
	time(&end);
	tdiff = difftime(end,start);
	dprintf(MSG_COMPLETED_AFTER_SECONDS, tdiff);
	dprintf(MSG_REBOOTING_IN);
	for (int i = 5; i > 0; i--)
	{
#ifdef USE_UNICODE
		dprintf(L"%i", i);
#else
		dprintf("%i", i);
#endif
		for (int j = 0; j < 4; j++)
		{
			Sleep(250);
#ifdef USE_UNICODE
			dprintf(L".");
#else
			dprintf(".");
#endif
		}
	}
	dprintf(MSG_BYE);
	HalReturnToFirmware(2);
}

VOID dumper(char *filename)
{
	if (!AutoMode)
	{
		if (fexists(filename))
		{
			dprintf(MSG_PRESS_START_TO_OVERWRITE_EXISTING_FILE, filename);
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
				dprintf(MSG_PRESS_A_TO_DUMP_SYSTEM_ONLY);
				dprintf(MSG_PRESS_B_TO_DUMP_FULL_NAND);
				dprintf(MSG_PRESS_BACK_TO_ABORT_DUMP);
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
						dprintf(MSG_TRY_AGAIN);
						dprintf(MSG_PRESS_A_TO_DUMP_SYSTEM_ONLY);
						dprintf(MSG_PRESS_B_TO_DUMP_FULL_NAND);
						dprintf(MSG_PRESS_BACK_TO_ABORT_DUMP);
					}
				}
			}
			else
			{
				if (size == (RAW_NAND_64*4))
					size = 256;
				else
					size = 512;
				dprintf(MSG_BB_DETECTED_SETTING_64MB, size);
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
	dprintf(MSG_COMPLETED_AFTER_SECONDS, tdiff);
	dumped = true;
}

void PrintExecutingCountdown(int max)
{
	for (; max > 0; max--)
	{
		dprintf(MSG_EXECUTING_COMMAND_IN_SECONDS, max);
		Sleep(1000);
	}
	dprintf(MSG_EXECUTING_COMMAND);
}

void TryAutoMode()
{
	dprintf(MSG_LOOKING_FOR_CMD_FILE_FOR_AUTO_MODE);
	if (fexists("game:\\simpleflasher.cmd"))
	{
		AutoMode = true;
		dprintf(MSG_SIMPLEFLASHER_CMD_FOUND_ENTERING_AUTO);
		int mode = CheckMode("game:\\simpleflasher.cmd");
		if (mode == 1) //AutoDump
		{
			dprintf(MSG_AUTO_DUMP_FOUND);
			dumper("game:\\flashdmp.bin");
			GenerateHash("game:\\flashdmp.bin");
		}
		else if (mode == 2) //AutoFlash
		{
			dprintf(MSG_AUTO_FLASH_FOUND);
			if (CheckHash("game:\\updflash.bin"))
			{
				PrintExecutingCountdown(5);
				flasher();
			}
			else
				dprintf(MSG_ERROR MSG_HASH_DONT_MATCH);
		}
		else if (mode == 3) //AutoSafeFlash
		{
			dprintf(MSG_AUTO_SAFE_FLASH_FOUND);
			if (CheckHash("game:\\updflash.bin"))
			{
				PrintExecutingCountdown(5);
				dumper("game:\\recovery.bin");
				AutoCountdown();
				flasher();
			}
			else
				dprintf(MSG_ERROR MSG_HASH_DONT_MATCH);
		}
		else if (mode == 4) //AutoExit, only want key...
		{
			dprintf(MSG_AUTO_EXIT_FOUND);
			PrintExecutingCountdown(5);
		}
		else if (mode == 5) //AutoReboot Hard Reset
		{
			dprintf(MSG_AUTO_REBOOT_FOUND);
			PrintExecutingCountdown(5);
			HalReturnToFirmware(2);
		}
		else
		{
			dprintf(MSG_BAD_COMMAND_FILE_RETURNING_TO_MANUAL_MODE);
			AutoMode = false;
			return;
		}
		if (AutoMode)
			XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP, 0); //We don't want to return to manual mode ;)
	}
	else
	{
		dprintf(MSG_COMMAND_FILE_NOT_FOUND_ENTERING_MANUAL_MODE);
	}
}

void PrintConsoleInfo(bool GotKey)
{
	dprintf(MSG_CONSOLE_INFO_LINE);
	PrintDash();
	PrintDLVersion();
	if (GotKey)
		PrintCPUKey();
	dprintf(MSG_CONSOLE_INFO_BOTTOM);
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

	
#ifdef TRANSLATION_BY
#ifdef USE_UNICODE
	dprintf(L"Simple 360 NAND Flasher by Swizzy v1.4b (BETA)\n");
#else
	dprintf("Simple 360 NAND Flasher by Swizzy v1.4b (BETA)\n");
#endif
	dprintf(TRANSLATION_BY);
#else
	dprintf("Simple 360 NAND Flasher by Swizzy v1.4b (BETA)\n\n");
#endif

	dprintf(MSG_DETECTING_NAND_TYPE);
	MMC = (sfcx_detecttype() == 1); // 1 = MMC, 0 = RAW NAND
	if (!MMC)
		config = sfcx_getconf();
	bool GotKey = false;
	dprintf(MSG_ATTEMTPING_TO_GRAB_CPUKEY);
	if (GetCPUKey())
	{
		SaveCPUKey("game:\\cpukey.txt");
		GotKey = true;
	}
	else
	{
		dprintf(MSG_ERROR MSG_INCOMPATIBLE_DASHLAUNCH);
		GotKey = false;
	}
	PrintConsoleInfo(GotKey);
	TryAutoMode();
	if (write)
	{
		if (!MMC)
		{
			dprintf(MSG_PRESS_A_TO_FLASH_RAWFLASH);
			dprintf(MSG_PRESS_B_TO_SAFE_FLASH_RAWFLASH);
		}
		else
		{
			dprintf(MSG_PRESS_A_TO_FLASH_RAWFLASH4G);
			dprintf(MSG_PRESS_B_TO_SAFE_FLASH_RAWFLASH4G);
		}
	}
	if (!MMC)
		dprintf(MSG_PRESS_X_TO_DUMP_RAWFLASH);
	else
		dprintf(MSG_PRESS_X_TO_DUMP_RAWFLASH4G);
	dprintf(MSG_PRESS_ANY_OTHER_BUTTON_TO_EXIT);
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
				dprintf(MSG_PRESS_ANY_BUTTON_TO_EXIT);
			}
			else if (m_pGamepad->wPressedButtons) { break; }
		}
		else if ((m_pGamepad->wPressedButtons) && (dumped)) { break; }
	}
	if (!MMC)
		sfcx_setconf(config);
}