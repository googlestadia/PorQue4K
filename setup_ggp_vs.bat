@REM Copyright 2020 Google Inc.
@REM 
@REM Licensed under the Apache License, Version 2.0 (the "License");
@REM you may not use this file except in compliance with the License.
@REM You may obtain a copy of the License at
@REM 
@REM http://www.apache.org/licenses/LICENSE-2.0
@REM 
@REM Unless required by applicable law or agreed to in writing, software
@REM distributed under the License is distributed on an "AS IS" BASIS,
@REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@REM See the License for the specific language governing permissions and
@REM limitations under the License.

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