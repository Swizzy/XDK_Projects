#ifndef __DEFAULT_TRANSLATION
#define __DEFAULT_TRANSLATION

#define MSG_SIMPLEFLASHER_CMD_FOUND_ENTERING_AUTO "game:\\simpleflasher.cmd Found!\n * Entering Automatic mode!\n"

#define MSG_GETTING_NAND_SIZE "Getting NAND Size...\n"
#define MSG_SIZE " * Size: 0x%X (%s)\n"

#define MSG_ERROR "\n ! ERROR: "

#define MSG_ERROR_READING_BYTES "\rerror reading 0x%X bytes at offset 0x%X (read: 0x%X bytes)!\n"
#define MSG_ERROR_WRITING_BYTES "\rerror writing 0x%X bytes at offset 0x%X (wrote: 0x%X bytes)!\n"
#define MSG_ERROR_FAILED_TO_READ_BYTES_FROM_FILE "\rERROR: failed to read %d bytes from file\n\n"
#define MSG_ERROR_FAILED_TO_WRITE_BYTES_TO_FILE "\rERROR: failed to write %d bytes to file\n\n"

#define MSG_ERROR_INVALID_FILE_SIZE "error: %s - size %d is not valid image size!\n"
#define MSG_ERROR_SIZE_NOT_SUPPORTED "error: size %d is not supported!\n"

#define MSG_FILE_NOT_FOUND "File %s not found\n"

#define MSG_UNABLE_TO_MOUNT_FLASH "Unable to mount Flash!"
#define MSG_UNABLE_TO_OPEN_FLASH_FOR_READING "Unable to open Flash: for reading\n"
#define MSG_UNABLE_TO_OPEN_FLASH_FOR_WRITING "Unable to open Flash: for writing\n"
#define MSG_UNABLE_TO_OPEN_FOR_WRITING "Unable to open %s for writing\n"
#define MSG_UNABLE_TO_OPEN_FOR_READING "Unable to open %s for reading, error: 0x%X\n"
#define MSG_UNABLE_TO_GET_SIZE_OF_FLASH "Unable to get size of flash!\n"
#define MSG_UNABLE_TO_WRITE_BYTES_TO_FLASH "Unable to write 0x%X bytes to flash, ERROR: 0x%X Offset: 0x%X"
#define MSG_UNABLE_TO_READ_FROM "Unable to read 0x%X bytes from %s (read: 0x%X bytes)\n"
#define MSG_UNABLE_TO_ALLOCATE_BUFFER_FOR "Unable to allocate 0x%X bytes buffer for %s\n"
#define MSG_UNABLE_TO_ALLOCATE_BUFFER "Unable to allocate 0x%X bytes for a buffer!\n"
#define MSG_UNABLE_TO_READ_0X630_BYTES_FROM "Unable to read 0x630 bytes from %s!\n"

#define MSG_PROCESSING_BLOCK "\rprocessing block 0x%X of 0x%X (%3.2f %%)"
#define MSG_PROCESSED_OF_BYTES "\rProcessed 0x%X of 0x%X bytes (%3.2f %%)"
#define MSG_PROCESSING_START "\rprocessing block"
#define MSG_PROCESSED_START "\rProcessed"

#define MSG_DONE_SUCCESS_DUMPED_BYTES "\nDone! successfully dumped 0x%X bytes to %s\n"
#define MSG_DONE_READ_BYTES_FROM_NAND_WROTE "\nDone! read 0x%X bytes from NAND, wrote 0x%X bytes to %s\n"
#define MSG_DONE_SUCCESS_WROTE_BYTES "Successfully wrote 0x%X bytes from %s to Flash!\n"
#define MSG_DONE_FAILED_WRITE_BYTES "Failed to write 0x%X bytes from %s to Flash!\n"

#define MSG_IMAGE_WRITTEN "image written :D\n"
#define MSG_IMAGE_DUMPED "NAND Dumped! :D\n"
#define MSG_FAILED_TO_WRITE_IMAGE "failed to write image :(\n"
#define MSG_FAILED_TO_DUMP_NAND "failed to dump NAND :(\n"

#define MSG_OPENED_OK_ATTEMPTING_READ_FROM_FLASH "%s opened OK, attempting to read 0x%X bytes from flash...\n"
#define MSG_OPENED_OK_ATTEMPTING_READ_FROM_FLASH_READABLE "%s opened OK, attempting to read 0x%X bytes (%s) from flash...\n"
#define MSG_OPENED_OK_ATTEMPTING_WRITE_TO_FLASH "%s opened OK, attempting to write 0x%X bytes to flash...\n"

#define MSG_READING_TO_MEMORY " * Reading in Image to Memory...\n"
#define MSG_IS_BIGGER_THEN_FLASH "%s is bigger then Flash! Aborting!\n"
#define MSG_IS_SMALLER_THEN_FLASH "%s is smaller then Flash! Aborting!\n"
#define MSG_WRITING_TO_FLASH " * Writing image to Flash...\n"

#define MSG_SFCX_ERROR "\r ! SFCX: "
#define MSG_BAD_BLOCK_FOUND_AT "Bad block found at 0x%X\n"
#define MSG_ILLEGAL_LOGICAL_BLOCK "Illegal logical block at 0x%X (status: 0x%X)\n"
#define MSG_UNKNOWN_ERROR_AT_ADDRESS "Unknown error at address 0x%X: 0x%X. Please worry.\n"
#define MSG_UNEXPECTED_SFCX_WRITE_PAGE_STATUS "Unexpected sfcx_write_page status 0x%X\n"

#define MSG_BLOCK_SEEMS_BAD "\rblock 0x%X seems bad, status 0x%X\n"
#define MSG_ATTEMPTING_BLOCK_RECOVERY "Attempting to recover the block...\n"
#define MSG_BLOCK_RECOVERED_SUCCESSFULLY "Block recovered! (A.K.A The block wasn't bad in the first place...)\n"
#define MSG_BLOCK_CANNOT_BE_RECOVERED "Block cannot be recovered! (A.K.A it's really bad)\n"
#define MSG_SFCX_UNSUPPORTED_TYPE_A0 "Unsupported Type A-0\n"
#define MSG_SFCX_UNSUPPORTED_TYPE_B0 "Unsupported Type B-0\n"
#define MSG_SFCX_UNSUPPORTED_TYPE "Unsupported Type\n"

#define MSG_RAW_NAND_DETECTED " * Detected RAW NAND device!\n * Entering NAND Mode...\n\n"
#define MSG_MMC_NAND_DETECTED " * Detected MMC NAND device!\n * Entering MMC/Corona v2/v4/v6 [4GB] Mode...\n\n"

#define MSG_CALCULATED_VALUE "\nCalculated value: %08X"
#define MSG_EXPECTED_VALUE "\nExpected value:   %08X"
#define MSG_CHECKING_CRC32 " * Checking %s Crc32: "
#define MSG_CALCULATING_CRC32_FOR "Calculating CRC32 for: %s\n"
#define MSG_ATTEMPTING_TO_SAVE_CRC32_TO " * Attempting to save the CRC32 value to %s...\n"
#define MSG_SUCCESSFULLY_SAVED_CRC32 " * successfully saved the CRC32 value!\n"

#define MSG_LOOKING_FOR " * Looking for %s... "
#define MSG_FOUND_CHECKING_CRC32 "Found!\n * Checking CRC32...\n"
#define MSG_NOT_FOUND "NOT Found!\n"

#define MSG_CURRENT_DASHBOARD "Current Dashboard: %d.%d.%d.%d\n"
#define MSG_CURRENT_DASHLAUNCH "Current DashLaunch version: %hu.%02hu (%hu)\n"
#define MSG_DASHLAUNCH_NOT_RUNNING "Current DashLaunch version: Not Running\n"
#define MSG_SYSCALL_RETURN_VALUE "SysCall Return value: 0x%llX\n" // Should probably be left alone as it has no use for anyone that isn't a developer whom wish to solve problems with this tool...
#define MSG_YOUR_CPUKEY_IS "Your CPUKey is: "
#define MSG_CPUKEY_SUCCESSFULLY_SAVED " * CPUKey successfully saved to: %s\n"

#define MSG_CHECKING_FOR_SPARE "Checking %s for spare data... "
#define MSG_BAD_MAGIC "%s Doesn't contain the magic bytes expected for an Xbox 360 NAND image, aborting!\n"
#define MSG_SPARE_DETECTED "Spare data detected!\n"
#define MSG_SPARE_NOT_DETECTED "No spare data detected!\n"

#define MSG_YOU_HAVE_SECONDS_BEFORE_CONTINUE "\rYou have %d Seconds to power off your console before the code continues..."
#define MSG_TIMES_UP "Time is up! Hope you didn't make a misstake! ;)\n"

#define MSG_PRESS_START_TO_FLASH "Press Start to flash your nand or press anything else to exit!\n\n"
#define MSG_PRESS_START_TO_OVERWRITE_EXISTING_FILE " **** WARNING: %s already exists! ****\nPress Start if you want to overwrite this file now!\n"
#define MSG_PRESS_A_TO_DUMP_SYSTEM_ONLY "Press A to Dump System Partition only (64MB, Recommended!)\n"
#define MSG_PRESS_B_TO_DUMP_FULL_NAND "Press B to Dump Full NAND (256MB/512MB, this may take a while...)\n"
#define MSG_PRESS_BACK_TO_ABORT_DUMP "Press Back to abort dump process...\n"

#define MSG_WARNING_DO_NOT_TOUCH_CONSOLE_OR_CONTROLLER "WARNING! DO NOT TOUCH YOUR CONSOLE OR CONTROLLER DURING THE FLASH PROCESS!!\nThe console will reboot when it's done!\n\n"
#define MSG_WARNING_YOU_ARE_ABOUT_TO_FLASH_NO_SPARE_TO_SPARE_CONSOLE "\n\nWARNING! You are about to flash an image that don't contain SPARE data to a machine that requires it!\nIf you know what you are doing Press Start to continue or anything else to exit!\n"
#define MSG_WARNING_YOU_ARE_ABOUT_TO_FLASH_SPARE_TO_NO_SPARE_CONSOLE "\n\nWARNING! You are about to flash an image that contains SPARE data to a machine that don't require it!\nIf you know what you are doing Press Start to continue or anything else to exit!\n"
#define MSG_WARNING_YOU_ARE_ABOUT_TO_FLASH_NO_SPARE_TO_SPARE_CONSOLE_AUTO "\n\nWARNING! You are about to flash an image that don't contain SPARE data to a machine that requires it!\nPower off your console to abort!"
#define MSG_WARNING_YOU_ARE_ABOUT_TO_FLASH_SPARE_TO_NO_SPARE_CONSOLE_AUTO "\n\nWARNING! You are about to flash an image that contains SPARE data to a machine that don't require it!\nPower off your console to abort!"

#define MSG_COMPLETED_AFTER_SECONDS "Completed after %4.2f seconds\n"
#define MSG_REBOOTING_IN "Rebooting in "
#define MSG_BYE "BYE!"
#define MSG_TRY_AGAIN "Try again:\n"

#define MSG_BB_DETECTED_SETTING_64MB " * %d MB NAND Detected! Setting dump size to 64MB!"

#define MSG_EXECUTING_COMMAND_IN_SECONDS "\rExecuting command in %d seconds"
#define MSG_EXECUTING_COMMAND "\rExecuting command!\n"

#define MSG_LOOKING_FOR_CMD_FILE_FOR_AUTO_MODE " * Looking for simpleflasher.cmd for automatic features...\n"
#define MSG_AUTO_DUMP_FOUND " * AutoDump command found!\n * Executing command!\n\n"
#define MSG_AUTO_FLASH_FOUND " * AutoFlash command found!\n\n"
#define MSG_AUTO_SAFE_FLASH_FOUND " * AutoSafeFlash command found!\n\n"
#define MSG_AUTO_EXIT_FOUND " * AutoExit command found!\n\n"
#define MSG_AUTO_REBOOT_FOUND " * AutoReboot command found!\n\n"
#define MSG_HASH_DONT_MATCH "Hash don't match, or file don't exist... Aborting!\n"
#define MSG_BAD_COMMAND_FILE_RETURNING_TO_MANUAL_MODE "! ERROR: Bad command file please read the readme!\nReturning to manual mode!\n"
#define MSG_COMMAND_FILE_NOT_FOUND_ENTERING_MANUAL_MODE "game:\\simpleflasher.cmd NOT Found!\n * Entering Manual mode!\n\n"

#define MSG_CONSOLE_INFO_LINE   "\n============= Console information: =============\n\n"
#define MSG_CONSOLE_INFO_BOTTOM "\n================================================\n\n"

#define MSG_DETECTING_NAND_TYPE " * Detecting NAND Type...\n"
#define MSG_ATTEMTPING_TO_GRAB_CPUKEY " * Attempting to grab CPUKey...\n"
#define MSG_INCOMPATIBLE_DASHLAUNCH "Incompatible Dashlaunch version detected! Use XeLL instead!\n"

#define MSG_PRESS_A_TO_FLASH_RAWFLASH "Press A if you want to flash your nand with Rawflash v5\n"
#define MSG_PRESS_A_TO_FLASH_RAWFLASH4G "Press A if you want to flash your nand with Rawflash4G v1\n"
#define MSG_PRESS_B_TO_SAFE_FLASH_RAWFLASH "Press B if you want to safeflash your nand with Rawflash v5 (Dump + Write)\n"
#define MSG_PRESS_B_TO_SAFE_FLASH_RAWFLASH4G "Press B if you want to safeflash your nand with Rawflash4G v1 (Dump + Write)\n"
#define MSG_PRESS_X_TO_DUMP_RAWFLASH "Press X if you want to dump your nand with Rawdump v1\n"
#define MSG_PRESS_X_TO_DUMP_RAWFLASH4G "Press X if you want to dump your nand with Rawdump4G v1\n"
#define MSG_PRESS_ANY_OTHER_BUTTON_TO_EXIT "If you press anything else the application will close...\n"
#define MSG_PRESS_ANY_BUTTON_TO_EXIT "Press any button to exit!\n"

#define MSG_PRESS_B_TO_OVERWRITE_EXISTING_FILE_SERIAL "Or press B to save/overwrite as %s_%s.bin\n"
#define MSG_ATTEMPTING_TO_GET_CONSOLE_SERIAL " * Attempting to get Console Serial...\n"
#define MSG_CONSOLE_SERIAL "Console Serial: %s\n"
#define MSG_GAME_NOT_MOUNTED_TRYING_USB "game:\\ isn't mounted, checking if a usb device is available...\n"
#define MSG_GAME_NOT_MOUNTED_TRYING_HDD "game:\\ isn't mounted, checking if a hdd is available...\n"
#define MSG_GAME_NOT_MOUNTED "game:\\ isn't mounted, unable to continue!\n"

#define MSG_READ_ONLY_RETURNING_TO_MANUAL_MODE "NAND flashing is disabled in this build.\nReturning to Manual Mode!\n" 
#define MSG_READ_ONLY_NOTICE "Notice: NAND flashing is disabled in this build\n"

#endif
