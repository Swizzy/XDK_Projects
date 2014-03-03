Simple 360 NAND Flasher By Swizzy v1.4b (BETA)
**********************************************

** Please note that this is an open BETA, it have been tested by me and a few friends, but... i don't take any
responsibility for possible bricks, make sure you understand this before you use this new version! **

****************
* Requirements *
****************

- A Hacked Xbox 360 (JTAG/RGH or similar)
- A nand dump to write or just enough space to save your nanddump on whatever device you use this app on
** For AutoMode: **
- A CRC32 hash of updflash.bin (in file updflash.crc32)
- A File named "simpleflasher.cmd" containing a command described in the secton "AutoMode" below 

****************
* What it does *
****************

- It'll flash your motherboards nand with the supplied image (updflash.bin) using rawflash v4 or rawflash4g v1
- It'll dump your motherboards nand with to flashdmp.bin using rawdump v1 or rawdump4g v1

************
* AutoMode *
************
Below is a list of commands followed by an explanation of what it does:
read - Dumps nand to flashdmp.bin and generates a CRC32 hash in flashdmp.crc32
dump - ^ same as above
write - Writes updflash.bin to nand (if CRC32 hash matches)
flash - ^ same as above
safe - Safeflash (dump + write) requires CRC32 hash like write, but dumps to recovery.bin instead
exit - Just exit the app, mainly meant for just extracting CPUKey...
reboot - Reboots the whole console, mainly meant for FSD plugin updates

In order for these features to actually work you have to write any of the above keywords/commands in a file called "simpleflasher.cmd"
This file is read on launch and deleted once read (NOTE: It'll ONLY read 1 line!)

*********************
* CPUKey Extraction *
*********************
It's finally here, i could've had it working a long time ago, i just didn't get the test app working... and decided to put it aside for a while...

This feature requires that you have dashlaunch 3.01 or later installed, if you don't it'll warn you about it...

***********
* Credits *
***********
- cOz Thanks for all the help you constantly give me with just about everything, and thanks for rawflash! :D
- Thanks to xvistaman for helping me solve that one last error i had with corona4g writes! i owe you ;)
- Thanks to blaKcat for listening to me about security stuff, CRC32 hashes etc. and not releasing the modified version before my official one ;)
- Thanks to everyone that has helped me test it!: Jonblu @ Consoleopen, Sileandro and Razkar And others that i don't know the name of...
- Thanks also goes to who ever it was that ported rawflash to SDK in the first place and making it open source!
- Thanks also to everyone that reports bugs and/or errors in ALL my apps :)

*******************
* Future features *
*******************
- Standalone Network controls (AutoMode was designed for AutoGG's network updates and requires FTP access with App Execution)

*************
* Changelog *
*************
v1.4b (BETA)
 - Fixed: It'll now properly show you when it detected a bad block that could not be recovered, if it could be recovered it'll mention that aswell
 - Fixed: Some output on screen wasn't saved to the log (apart from the progress indication which is quite useless)
 - Fixed: Some messages would be added after the progress indication while in NAND mode instead of overwriting the progress indication (it's now shown correctly and logged!)

v1.4 (BETA)
 - Changed: CPUKey Extraction now also works with Dashlaunch 3.08 and later
 - Changed: The message shown when CPUKey Extraction fails nolonger mentions old dashboards, but instead mentions incompatible Dashlaunch version...
 - Changed: Rawflash now tries to recover from a previous badflash (it erases each block and checks it again before thinking of it as bad...)
 - Added: It'll now also show you the currently running Dashlaunch version (if there is one running)

v1.3 (BETA)
 - Added: Auto/Manual mode, it can now do things straight away rather then waiting for user input...
 - Added: CPUKey Extraction this is done before it asks what to do, or even reads automode files...
 - Added: It'll display the currently running dashboard version for you (Useful when dumping/updating to see what's running atm)
 - Fixed: Output log will nolonger include anything that starts with "\r" meaning anything that reports status...
 - Changed: Changed the text color to gold (0xFFFFD700)
 - Changed: Font size is now smaller, and i've changed the font from Verdana to X360 by Redge
 - Changed: The console will now hard-reset itself rather then power off (same way as a normal Xbox 360 update do)
 - Changed: It'll now clear the screen before it starts reading/writing, giving you a more clear picture of what's going on ;)

v1.2
 - Added: Full support for Corona v2 (4GB Corona)
 - Added: Auto Detect which mode to run in (it'll autodetect if you are running on a MMC type of machine or a RAW type [MMC type = Corona 4GB/V2, RAW = any other older machine])
 - Added: Safe-Flash (Read to recovery.bin then write)
 - Added: Autoshutdown after flash (it'll tell you it's shutting down then count down starting at 5 seconds...)
 - Added: It'll turn off your wireless controllers during flash to keep you safe (if you reconnect them they'll be shut off again while it's working)
 - Fixed: If you accidently hit eject the app now stays running! :D
 - Fixed: You no longer see the options to write if you don't have a updflash.bin next to the app...
 - Fixed: Lot's of code cleanup, making it look nicer and easier to follow/understand :)
 - Added: It'll now also check for Spare data before writing, again it's a safety measure, if it finds spare data while in MMC mode it'll warn you about it and give you the option to exit... and of course the other way around for raw mode...
 - Added: The above check for spare also checks for the "Magic bytes" (your Xbox 360 image should start with 0xFF4F, i've never seen an image that differ from this, and thus it's an easy way for me to check that the file you give the app really is a nand...)
 - Fixed: The app will now clear the line of output before it starts overwriting, this means that you'll no longer see leftover output on screen from previous output ;) (mostly for error messages during progress output)
 - Changed: The app now use a different font (it used Arial before, now it uses verdana)

** WARNING!! If you don't let the app exit after flashing a corona v2 you will most likely end up with a machine that no longer boots untill you reflash it using a computer!! You've been warned... **
** Altought this version adds multipile security checks before it starts writing an image to your nand it can still be very dangerous... if you don't know what you are doing, don't do it! ;) **

v1.1
 - Added: Now able to dump your nand
 - Added: Now also writes a log of whatever output it gave to you on screen (except for current progress output)
 - Added: Timer, it now checks and tell you how long it took to perform the task (in seconds... if there is more then 60, you'll have to calculate on your own how many minutes and so on it took ;))
 - Fixed: Now properly turns of when you've flashed your NAND
** A Little word of warning: DO NOT exit the app after flashing... turn of your console after flashing it!! or you'll not be particularly happy with what you see ;) **

v1.0
 - Initial release, Just a quick mod of the previously released "nandflasher_xex" to fix output issue/problem and make it slightly more "safe" by requiring input from you...