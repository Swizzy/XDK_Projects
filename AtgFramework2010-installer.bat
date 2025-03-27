@echo off
cd /d "%~dp0"

# The 'Common' folders in each project subdirectory contain modified files from XDK example code. We can't include unmodified files from the XDK examples, so this script doesn't overwrite any of the modified files and only adds what is missing.
# For 32 bit systems %ProgramFiles(x86)% doesn't exist so we need to check for both. 

if exist "%ProgramFiles(x86)%\Microsoft Xbox 360 SDK\Source\Samples\Common\" (
    echo n | copy /-y "%ProgramFiles(x86)%\Microsoft Xbox 360 SDK\Source\Samples\Common\*" "Simple 360 NAND Flasher\Common"
    echo n | copy /-y "%ProgramFiles(x86)%\Microsoft Xbox 360 SDK\Source\Samples\Common\*" "NXE2GOD\Common"
) else if exist "%ProgramFiles%\Microsoft Xbox 360 SDK\Source\Samples\Common\" (
    echo n | copy /-y "%ProgramFiles%\Microsoft Xbox 360 SDK\Source\Samples\Common\*" "Simple 360 NAND Flasher\Common"
    echo n | copy /-y "%ProgramFiles%\Microsoft Xbox 360 SDK\Source\Samples\Common\*" "NXE2GOD\Common"
) else (
    echo Error: Cannot find the XDK! Make sure it is installed.
)

pause
