NXE2GOD 1.3
-----------

***************
* What is it? *
***************
NXE2GOD is a tool to convert NXE installs (Game Disc installs in NXE or newer) to GOD (Games On Demand)
Basically what this means is that we remove the need for the disc to be in the DVD Drive to play the game :)

***********
* Credits *
***********
- Dstruktiv - Thanks for making the original and releasing your source code! :D
- Anyone else that has helped with the project in any way ;)

*****************
* Install Notes *
*****************

In the release there is 2 packages, one for XEX and another for LIVE, the LIVE one is meant for installs
to device:\Content\0000000000000000\FFFF0055\00080000\ which allows you to start it straight from dash! :D

The XEX Version is meant for those of you wanting to create your own live package, or simply wish to run it from USB ;)

*************
* Changelog *
*************
v1.3 - Mod by Swizzy released 24-05-2015
+ Added support for USB devices (since USBMU after 17349 actually go into USB#:\Content\..... as a hidden directory)

v1.2 - Mod by Swizzy released 23-01-2013
+ Added support for USBMU Devices, Limited to: USBMU0, USBMU1, USBMU2, USBMU3, USBMU4
(Meaning up to 5 USB Devices with a Memory unit on it) still have the HDD scan tho ;)
+ Fixed bug with leaving the 00004000 directory, it'll now be deleted like everything else
+ Font is now embedded, no need for Media folder and shit ;)
- Removed backup crap, if you need it, you can read the original source code and add it back ;)
+ Lot's of code cleanup!

V1.1
+Fixed bug where some games would potentially not convert properley due to
files not being moved to the correct place.

+Minor speed increase

V1.0:
+Fixed issue where if you had more than one game with the same titleid, then
converting the games would result in all games using that titleid being deleted
except for one.

+Now have 100% compatibility with ALL games.

+Improved interactive interface with useful output. 

V0.2:
+Games converted with NXE2GOD will now be useable on any homebrew console, they
are no longer locked to the console they were ripped on.

Converts NXE installed games to Games on Demand.
