# VulkanPorQue4K
4K yee

# Builds

## Build Windows
```
$ mkdir buildWinVS2017
$ cd buildWinVS2017
$ cmake -G "Visual Studio 15 2017 Win64" -DDXC_PATH="C:\src\sc\dxc\d0e9147a\bin\dxc.exe" ..
```

Then build from generated Visual Studio solution `buildWinVS2017\VulkanPorQue4K.sln`

## Build GGP

Deploy assets to binary (from repo root folder):
```
> ggp ssh put -r ./assets
```

TODO: This should be part of an initial setup script

### Build GGP + Ninja
```
$ mkdir buildGGP
$ cd buildGGP
$ cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$GGP_SDK_PATH/cmake/ggp.cmake" -DCMAKE_BUILD_WITH_INSTALL_RPATH=TRUE -DDXC_PATH="C:\src\sc\dxc\d0e9147a\bin\dxc.exe"
$ ninja
```

Then 4KApp binary to gamelet
```
> ggp ssh put ./buildGGP/src/app/4KApp
> ggp run --cmd "4KApp"
```

### Build GGP + Visual Studio
Right now, it's not entirely clear how to use CMake to directly generate a GGP-compatible
Visual Studio solution. However, we can get pretty close, and fix up the last deltas with
a custom script. Obviously, this is fragile, but it works ok for now.

```
$ mkdir buildGGPVS
$ cd buildGGPVS
$ cmake -G "Visual Studio 15 Win64" -DDXC_PATH="C:\src\sc\dxc\fdd4e3b0\bin\dxc.exe" .. -DCMAKE_TOOLCHAIN_FILE="$GGP_SDK_PATH/cmake/ggp.cmake"
$ python3 ../ggpvs_conversion.py3 .
```

Then open `buildGGPVS/VulkanPorQue4K.sln` with Visual Studio 2017. Building should work, though
getting the debugging working is still TBD (not deploying binary correctly yet). 

## Build Linux
TBD - Help!