@echo off
setlocal enableDelayedExpansion enableExtensions

cls
title ^|======================^|
echo.    []======================================[]
echo. 
echo.For complete pictures:
echo. 
echo. 1. Maximize this window;
echo. 2. Scoll the mouse wheel with Ctrl pressed, or use
echo.    the trackpad or touch screen to zoom the text,
echo.    making the two lines above near in length.
echo. 
echo.Press any key to start.
pause>nul
echo.

if not exist "%~dp0imglist.txt" dir /b "%~dp0*.jpg">"%~dp0imglist.txt"
set params=":%~dp0imglist.txt" --interval=66 "--media=%~dp0wave.mp3" %*
for /f tokens^=2 %%L in ('mode con^|find "Col"') do set params=!params! --max-width=%%~L

title Where do you want to go today? ('95)
echo start "" /d "%~dp0" /wait /b "%~dp0..\make\x64\Release\saam.exe" %params%

timeout 30