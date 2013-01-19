#include "AtgConsole.h"
#include "AtgInput.h"
#include "AtgUtil.h"
#include <time.h>
#include <sys/stat.h>
#include "Corona4G.h"

extern "C" {
#include "xenon_sfcx.h"
}

ATG::Console    g_console;            // console for output
ATG::GAMEPAD*	m_pGamepad; // Gamepad for input
time_t start,end; //Timer times for measuring time difference
double tdiff; // Double for time difference
char buf[512]; //Text buffer
bool started = false, dumped = false, dump_in_progress = false, MMC = false, write = false;
unsigned int config = 0;

extern "C" VOID __cdecl dprintf( const CHAR* strFormat, ... )
{
	FILE* flog;
	va_list pArglist;
	va_start( pArglist, strFormat );
	vsnprintf_s(buf, 512, strFormat, pArglist);
	va_end( pArglist );
	fopen_s(&flog, "game:\\Simple 360 NAND Flasher.log", "a");
	if (flog != NULL)
	{
		const CHAR* ignore = "processing block";
		const CHAR* ignore2 = "Processed";
		if (strncmp(strFormat, ignore, strlen(ignore)) != 0)
			if (strncmp(strFormat, ignore2, strlen(ignore2)) != 0)
				fprintf(flog, buf);
		fclose(flog);
	}
	g_console.Display(buf);
}

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

bool fexists(const char *filename)
{
	struct stat info;
	if (stat(filename, &info) == 0)
		return true;
	return false;
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

VOID flasher()
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
	started = true;
	time(&start);
	dprintf("WARNING! DO NOT TOUCH YOUR CONSOLE OR CONTROLLER DURING THE FLASH PROCESS!!\nThe console will power off when it's done!\n\n");
	int tmp = HasSpare("game:\\updflash.bin");
	if (tmp == -1)
		XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP, 0);
	bool isEcced = (tmp == 0);	
	if (!MMC)
	{
		if (!isEcced)
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
		KillControllers();
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
		KillControllers();
		try_rawflash4g("game:\\updflash.bin");
	}
	time(&end);
	tdiff = difftime(end,start);
	dprintf("Completed after %4.2f seconds\n", tdiff);
	dprintf("Shutting down in ");
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
	HalReturnToFirmware(5);
}
VOID dumper(char *filename)
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
	started = true;
	if (!MMC)
	{
		sfcx_init();
		sfcx_setconf(config);
		int size = sfc.size_bytes_phys;
		if((size == (RAW_NAND_64*4)) || (size == (RAW_NAND_64*8)))
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
					//dprintf("Try again: Press A to Dump System Partition only (64MB, Recommended!) or\nPress B to dump Full NAND (256MB/512MB, this may take a while...)\n\n");					
					dprintf("Try again:\n");
					dprintf("Press A to Dump System Partition only (64MB, Recommended!)\n");
					dprintf("Press B to Dump Full NAND (256MB/512MB, this may take a while...)\n");
					dprintf("Press Back to abort dump process...\n");
				}
			}
		}
		time(&start);
		unsigned int r = sfcx_init();
		sfcx_printinfo(r);
		try_rawdump(filename, size);
		sfcx_setconf(config);
	}
	else
	{
		time(&start);
		try_rawdump4g(filename);
	}
	time(&end);
	tdiff = difftime(end,start);
	dprintf("Completed after %4.2f seconds\n", tdiff);
	dumped = true;
}

//--------------------------------------------------------------------------------------
// Name: main()
// Desc: Entry point to the program
//--------------------------------------------------------------------------------------
VOID __cdecl main()
{
	// Initialize the console window
	write = fexists("game:\\updflash.bin");
	g_console.Create("embed:\\font", 0xFF000000, 0xFF008000);
	g_console.SendOutputToDebugChannel( TRUE );

	dprintf("Simple 360 NAND Flasher by Swizzy v1.2\n\n");

	dprintf("Detecting NAND Type...\n");
	MMC = (sfcx_detecttype() == 1); // 1 = MMC, 0 = RAW NAND

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

	if (!MMC)
		config = sfcx_getconf();
	for(;;)
	{
		g_console.Render();
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