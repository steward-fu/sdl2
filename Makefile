SDL2_CFG = --disable-joystick-virtual
SDL2_CFG+= --disable-jack
SDL2_CFG+= --disable-power
SDL2_CFG+= --disable-sensor
SDL2_CFG+= --disable-ime
SDL2_CFG+= --disable-dbus
SDL2_CFG+= --disable-fcitx
SDL2_CFG+= --disable-hidapi
SDL2_CFG+= --disable-libudev
SDL2_CFG+= --disable-video-x11
SDL2_CFG+= --disable-video-kmsdrm
SDL2_CFG+= --disable-video-vulkan
SDL2_CFG+= --disable-video-opengl
SDL2_CFG+= --disable-video-opengles
SDL2_CFG+= --disable-video-opengles2
SDL2_CFG+= --disable-video-wayland
SDL2_CFG+= --disable-video-dummy
SDL2_CFG+= --disable-oss
SDL2_CFG+= --disable-alsa
SDL2_CFG+= --disable-sndio
SDL2_CFG+= --disable-diskaudio
SDL2_CFG+= --disable-pulseaudio
SDL2_CFG+= --disable-dummyaudio

MOD      = mmiyoo
REL_VER  = $(shell git rev-parse HEAD | cut -c 1-8)
SDL2_INC = -I/opt/mmiyoo/arm-buildroot-linux-gnueabihf/sysroot/usr/include/SDL2
GPU_CMD  = /opt/mmiyoo/bin/cmake -DARCH=arm --host=/opt/mmiyoo/share/buildroot/toolchainfile.cmake ..

ifeq ($(MOD),mmiyoo)
    SDL2_CFG+= --disable-oss
    SDL2_CFG+= --disable-alsa
    export CROSS=/opt/mmiyoo/bin/arm-linux-gnueabihf-
endif

ifeq ($(MOD),trimui)
    SDL2_CFG+= --disable-oss
    SDL2_CFG+= --disable-alsa
    export CROSS=/opt/mmiyoo/bin/arm-linux-gnueabihf-
endif

ifeq ($(MOD),funkeys)
    export CROSS=/opt/mmiyoo/bin/arm-linux-gnueabihf-
endif

ifeq ($(MOD),pandora)
    SDL2_CFG+= --disable-oss
    SDL2_CFG+= --disable-alsa
endif

ifeq ($(MOD),qx1000)
    SDL2_CFG+= --disable-oss
    SDL2_CFG+= --disable-alsa
	GPU_CMD  = cmake -DARCH=arm ..
	SDL2_INC = -I/usr/include/SDL2
endif

ifeq ($(MOD),unittest)
    SDL2_CFG+= --disable-oss
    SDL2_CFG+= --disable-alsa
    export MOD=unittest
    $(shell cd sdl2 && rm -rf libEGL.so libGLESv2.so)
else
    export CC=${CROSS}gcc
    export AR=${CROSS}ar
    export AS=${CROSS}as
    export LD=${CROSS}ld
    export CXX=${CROSS}g++
    export HOST=arm-linux
    $(shell cd sdl2 && rm -rf libEGL.so libGLESv2.so)
    $(shell cd sdl2 && ln -s ../swiftshader/build/libEGL.so)
    $(shell cd sdl2 && ln -s ../swiftshader/build/libGLESv2.so)
endif

.PHONY: all
all: gpu sdl2 example

.PHONY: sdl2
sdl2:
	make -C sdl2 -j4

.PHONY: gpu
gpu:
	cd swiftshader/build && make -j4

.PHONY: example
example:
	cp assets/$(MOD)/* example/
	cp swiftshader/build/*.so example/libs/
	cp sdl2/build/.libs/libSDL2-2.0.so.0 example/libs/
	$(CC) example/main.c $(SDL2_INC) -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_gfx -lSDL2_ttf swiftshader/build/libGLESv2.so -o example/test
	$(CXX) example/gles.cpp -Wno-narrowing $(SDL2_INC) -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_gfx -lSDL2_ttf swiftshader/build/libGLESv2.so -o example/gles

.PHONY: cfg
cfg:
	cd sdl2 && ./autogen.sh && MOD=$(MOD) ./configure ${SDL2_CFG} --host=${HOST}
	cd swiftshader/build && ${GPU_CMD}

.PHONY: clean
clean:
	cd example && rm -rf test launch.sh config.json
	cd example/libs && rm -rf libEGL.so libGLESv2.so libSDL2-2.0.so.0
	make -C sdl2 distclean
