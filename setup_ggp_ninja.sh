#!/bin/bash

if [ $# -eq 0 ]; then
    echo "No DXC path supplied!"
    exit 1
fi

mkdir buildGGP
cd buildGGP
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$GGP_SDK_PATH/cmake/ggp.cmake" -DCMAKE_BUILD_WITH_INSTALL_RPATH=TRUE -DDXC_PATH=$1 ..
ninja

ggp ssh put ./src/app/4KApp