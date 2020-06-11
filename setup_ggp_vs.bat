rem Copyright 2020 Google Inc.
rem 
rem Licensed under the Apache License, Version 2.0 (the "License");
rem you may not use this file except in compliance with the License.
rem You may obtain a copy of the License at
rem 
rem http://www.apache.org/licenses/LICENSE-2.0
rem 
rem Unless required by applicable law or agreed to in writing, software
rem distributed under the License is distributed on an "AS IS" BASIS,
rem WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
rem See the License for the specific language governing permissions and
rem limitations under the License.

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