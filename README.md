# VulkanPorQue4K

4K yee  
We out here, vibin'

# Building

Before doing anything, make sure your submodules are up-to-date!
```
$ git submodule update --init --recursive
```

## Build for Windows

```
$ mkdir buildWinVS2017
$ cd buildWinVS2017
$ cmake -G "Visual Studio 15 2017 Win64" -DDXC_PATH="C:\src\sc\dxc\d0e9147a\bin\dxc.exe" ..
```

Then build from generated Visual Studio solution
`buildWinVS2017\VulkanPorQue4K.sln`

You can use the included `setup_windows.bat` batch script to generate the Visual
Studio 2017 solution + projects.
```
> setup_windows.bat <path to DXC binary>
```

## Build for GGP

### Deploy Assets

Deploy assets to GGP instance (from repo root folder):  
`deploy_assets.bat`|`deploy_assets.sh`
```
> ggp ssh put -r ./assets
```

### Build GGP + Ninja

```
$ mkdir buildGGP
$ cd buildGGP
$ cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$GGP_SDK_PATH/cmake/ggp.cmake" -DCMAKE_BUILD_WITH_INSTALL_RPATH=TRUE -DDXC_PATH="C:\src\sc\dxc\d0e9147a\bin\dxc.exe"
$ ninja
```

Then deploy the 4KApp binary to gamelet
```
> ggp ssh put ./buildGGP/src/app/4KApp
> ggp run --cmd "4KApp"
```

Alternatively, you can use `setup_ggp_ninja.sh` to build and deploy the binary
to your instance!
```
$ sh setup_ggp_ninja.sh <path to DXC binary>
```

### Build GGP + Visual Studio

Using a special fork of CMake, we're able to generate GGP-enabled Visual Studio
2017 solution + project files.You can clone the
https://github.com/chaoticbob/CMake repo, or grab a
[pre-built release](https://github.com/chaoticbob/CMake/releases/tag/cmake-ggp-6115824).

Once you have it, you can use this command line:
```
$ cmake -G "Visual Studio 15 2017" -T "Ggp_Clang" -A "GGP" -DCMAKE_TOOLCHAIN_FILE="$GGP_SDK_PATH/cmake/ggp.cmake" -DDXC_PATH="C:\src\sc\dxc\9c89a1c2\bin\dxc.exe" ..
```
You can use a batch script to automate this process:
`setup_ggp_vs.bat`
```
> mkdir buildGGPVS
> cd buildGGPVS
> cmake -G "Visual Studio 15 2017" -T "Ggp_Clang" -A "GGP" -DCMAKE_TOOLCHAIN_FILE="%GGP_SDK_PATH%/cmake/ggp.cmake" -DDXC_PATH="C:\src\sc\dxc\9c89a1c2\bin\dxc.exe" ..
```

Then open `buildGGPVS/VulkanPorQue4K.sln` with Visual Studio 2017. Building and
debugging should work (once you set the Application in the Debug properties).

## Build for Linux

TBD - Help!

# Running the Sample

## Windows / Linux

Currently only runs windowed, and supports 1080p and 2160p (more in the future).
```
4KApp --height 2160
```

## GGP

Swapchain resolution is detected during app initialization.
The sample currently doesn't support swapchain resolution change, because who
cares, that's not the point of the sample.

# Upscaling Techniques
See the [Techniques Hub document](docs/TECHNIQUES.md).

# Contributing
[Contribution guidelines for this project](docs/CONTRIBUTING.md).

# DirectXShaderCompiler Info
This project uses [DXC](https://github.com/microsoft/DirectXShaderCompiler) to
compile HLSL shaders to SPIR-V.

Currently, the project has been verified against
[9c89a1c2](https://github.com/microsoft/DirectXShaderCompiler/commit/9c89a1c2c6baa76dabc154f126408973848b0069).