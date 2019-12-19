@ECHO OFF

IF NOT "%~1"=="" IF "%~2"=="" GOTO START
ECHO This script requires the next parameters:
ECHO - Path to DXC binary
ECHO Example:
ECHO "%~nx0" "C:\path\to\dxc\bin\dxc.exe"
GOTO :EOF

:START

@ECHO ON

mkdir buildWinVS2017
cd buildWinVS2017
cmake -G "Visual Studio 15 2017 Win64" -DDXC_PATH=%~1 ..
