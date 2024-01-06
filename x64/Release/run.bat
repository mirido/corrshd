@echo off

rem Get the drive where the executable file is located.
set EXEDRV=%~d0

rem Identify OpenCV DLL folder.
set OPENCVDIR=%EXEDRV%\opencv\build
set OPENCVDLLDIR=%OPENCVDIR%\x64\vc16\bin

rem Make path to OpenCV DLL.
PATH=%OPENCVDLLDIR%;%PATH%

corrshd.exe D:\usr\document\IMGP0079.jpg 182 257 800
