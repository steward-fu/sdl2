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
	make -C sdl2

.PHONY: demo
demo:
	cp swiftshader/build/*.so $(MOD)/demo/libs/
	cp sdl2/build/.libs/libSDL2-2.0.so.0 $(MOD)/demo/libs/
	$(CC) $(MOD)/demo/main.c -I/opt/mmiyoo/arm-buildroot-linux-gnueabihf/sysroot/usr/include/SDL2 -lSDL2 -o $(MOD)/demo/demo

.PHONY: config
config:
	cd sdl2 && ./autogen.sh && MOD=$(MOD) ./configure ${SDL2_CFG} --host=${HOST}
	cd swiftshader/build && /opt/mmiyoo/bin/cmake -DARCH=arm --host=/opt/mmiyoo/share/buildroot/toolchainfile.cmake ..

.PHONY: clean
clean:
	rm -rf mmiyoo/demo/demo trimui/demo/demo
	rm -rf mmiyoo/demo/libs trimui/demo/libs
	rm -rf swiftshader/build
	mkdir -p mmiyoo/demo/libs trimui/demo/libs
	mkdir -p swiftshader/build
	make -C sdl2 distclean
