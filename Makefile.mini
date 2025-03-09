SDL2_CFG+= --disable-oss
SDL2_CFG+= --disable-alsa
SDL2_CFG+= --disable-video-opengl
SDL2_CFG+= --disable-video-opengles
SDL2_CFG+= --disable-video-opengles2
SDL2_CFG+= --enable-audio-mini
SDL2_CFG+= --enable-video-mini

SDL2_INC = -I/opt/mini/arm-buildroot-linux-gnueabihf/sysroot/usr/include/SDL2
GPU_CMD  = /opt/mini/bin/cmake -DARCH=arm --host=/opt/mini/share/buildroot/toolchainfile.cmake ..

export CROSS=/opt/mini/bin/arm-linux-gnueabihf-
export CC=${CROSS}gcc
export AR=${CROSS}ar
export AS=${CROSS}as
export LD=${CROSS}ld
export CXX=${CROSS}g++
export HOST=arm-linux

$(shell cd sdl2 && ln -s ../swiftshader/build/libEGL.so)
$(shell cd sdl2 && ln -s ../swiftshader/build/libGLESv2.so)

include Makefile.mk

.PHONY: all
all: gpu sdl2

.PHONY: gpu
gpu:
	cd swiftshader/build && make -j4

.PHONY: cfg
cfg:
	cd sdl2 && ./autogen.sh && ./configure ${SDL2_CFG} --host=${HOST}
	mkdir -p swiftshader/build && cd swiftshader/build && ${GPU_CMD}
