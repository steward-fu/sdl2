export CROSS=/opt/mmiyoo/bin/arm-linux-gnueabihf-
export CC=${CROSS}gcc
export AR=${CROSS}ar
export AS=${CROSS}as
export LD=${CROSS}ld
export CXX=${CROSS}g++
export HOST=arm-linux

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

.PHONY: all
all:
	cd swiftshader/build && make -j4
	make -C sdl2 -j4

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
	$(CC) example/main.c -I/opt/mmiyoo/arm-buildroot-linux-gnueabihf/sysroot/usr/include/SDL2 -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_gfx -lSDL2_ttf -lGLESv2 -o example/test

.PHONY: cfg
cfg:
	cd sdl2 && ./autogen.sh && MOD=$(MOD) ./configure ${SDL2_CFG} --host=${HOST}
	cd swiftshader/build && /opt/mmiyoo/bin/cmake -DARCH=arm --host=/opt/mmiyoo/share/buildroot/toolchainfile.cmake ..

.PHONY: clean
clean:
	cd example && rm -rf test launch.sh config.json
	cd example/libs && rm -rf libEGL.so libGLESv2.so libSDL2-2.0.so.0
	make -C sdl2 distclean
