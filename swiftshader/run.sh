#!/bin/bash
if [ ! -z "$1" ] && [ "$1" == "mmiyoo" ]; then
    export CROSS=/opt/mmiyoo/bin/arm-linux-gnueabihf-
    export CC=${CROSS}gcc
    export AR=${CROSS}ar
    export AS=${CROSS}as
    export LD=${CROSS}ld
    export CXX=${CROSS}g++
    export HOST=arm-linux

    echo "Build for Miyoo-Mini Handheld"
    mkdir -p mmiyoo
    cd mmiyoo
    cmake -DARCH=arm --host=/opt/mmiyoo/share/buildroot/toolchainfile.cmake ..
    make -j4
else
    echo "Build for Linux PC"
    mkdir -p x64
    cd x64
    cmake -DARCH=x86_64 ..
    make -j4
fi
