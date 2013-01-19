Simple 360 NAND Flasher By Swizzy v1.2
**************************************

****************
* Requirements *
****************

- A Hacked Xbox 360 (JTAG/RGH or similar)
- A nand dump to write or just enough space to save your nanddump on whatever device you use this app on

****************
* What it does *
****************

- It'll flash your motherboards nand with the supplied image (updflash.bin) using rawflash v4 or rawflash4g v1
- It'll dump your motherboards nand with to flashdmp.bin using rawdump v1 or rawdump4g v1

***********
* Credits *
***********
- cOz Thanks for all the help you constantly give me with just about everything, and thanks for rawflash! :D
- Thanks to xvistaman for helping me solve that one last error i had with corona4g writes! i owe you ;)
- Thanks to everyone that has helped me test it!: Jonblu @ Consoleopen, Sileandro and Razkar And others that i don't know the name of...
- Thanks also goes to who ever it was that ported rawflash to SDK in the first place and making it open source!
- Thanks also to everyone that reports bugs and/or errors in ALL my apps :)

*******************
* Future features *
*******************
 - Cpukey retreival:
I'm working on getting this to work, it's not present in the current source code, but i've got a few things to work with that will hopefully make this possible aswell, it might not work with all dashboard versions etc. but atleast the current ones ;) it's probably going to take a while before you see this feature, but have no fear, it'll come eventually ;)

*************
* Changelog *
*************

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