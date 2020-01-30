@ECHO OFF

IF NOT "%~1"=="" IF "%~2"=="" GOTO START
ECHO This script requires the next parameters:
ECHO - Path to DXC binary
ECHO Example:
ECHO "%~nx0" "C:\path\to\dxc\bin\dxc.exe"
GOTO :EOF

:START

@ECHO ON

mkdir buildGGPVS
cd buildGGPVS
cmake -G "Visual Studio 15 2017" -T "Ggp_Clang" -A "GGP" -DCMAKE_TOOLCHAIN_FILE="%GGP_SDK_PATH%/cmake/ggp.cmake" -DDXC_PATH=%~1 ..