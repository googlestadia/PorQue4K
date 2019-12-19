@ECHO OFF

IF NOT "%~2"=="" IF "%~3"=="" GOTO START
ECHO This script requires the next parameters:
ECHO - Path to DXC binary
ECHO - Path to Python3 binary
ECHO Example:
ECHO "%~nx0" "C:\path\to\dxc\bin\dxc.exe" "C:\path\to\python3\python.exe"
GOTO :EOF

:START

@ECHO ON

mkdir buildGGPVS
cd buildGGPVS
cmake -G "Visual Studio 15 Win64" -DDXC_PATH=%~1 -DCMAKE_TOOLCHAIN_FILE="%GGP_SDK_PATH%/cmake/ggp.cmake" ..
%~2 ../ggpvs_conversion.py3 .