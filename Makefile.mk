SDL2_CFG+= --disable-joystick-virtual
SDL2_CFG+= --disable-jack
SDL2_CFG+= --disable-power
SDL2_CFG+= --disable-ime
SDL2_CFG+= --disable-dbus
SDL2_CFG+= --disable-fcitx
SDL2_CFG+= --disable-hidapi
SDL2_CFG+= --disable-libudev
SDL2_CFG+= --disable-video-x11
SDL2_CFG+= --disable-video-kmsdrm
SDL2_CFG+= --disable-video-vulkan
SDL2_CFG+= --disable-video-wayland
SDL2_CFG+= --disable-video-dummy
SDL2_CFG+= --disable-sndio
SDL2_CFG+= --disable-diskaudio
SDL2_CFG+= --disable-pulseaudio
SDL2_CFG+= --disable-dummyaudio

REL_VER  = $(shell git rev-parse HEAD | cut -c 1-8)

.PHONY: sdl2
sdl2:
	make -C sdl2 -j4

.PHONY: example
example:
	cp assets/$(MOD)/* example/
	cp sdl2/build/.libs/libSDL2-2.0.so.0 example/lib/
	$(CC) example/sdl2.c $(SDL2_INC) -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_gfx -lSDL2_ttf -lGLESv2 -o example/sdl2
	$(CXX) example/opengles.cpp -Wno-narrowing $(SDL2_INC) -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_gfx -lSDL2_ttf -lGLESv2 -o example/opengles

.PHONY: clean
clean:
	cd example && rm -rf sdl2 opengles launch.sh config.json
	cd example/lib && rm -rf libMali.so libEGL.so libGLESv2.so libSDL2-2.0.so.0
	cd sdl2 && rm -rf libMali.so libEGL.so libGLESv2.so
	make -C sdl2 distclean
