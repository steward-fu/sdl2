// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#ifndef __SDL_XT894_VIDEO_H__
#define __SDL_XT894_VIDEO_H__

#if SDL_VIDEO_DRIVER_XT894

#include <wayland-egl.h>
#include <wayland-client.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "../SDL_sysvideo.h"

#if 0
    #define debug(...) printf(__VA_ARGS__)
#else
    #define debug(...) (void)0
#endif

#define XT894_DRIVER_NAME "XT894"

#define MAX_FB  2
#define LCD_W   540
#define LCD_H   960

struct _wayland {
    struct wl_shell* shell;
    struct wl_region* region;
    struct wl_display* display;
    struct wl_surface* surface;
    struct wl_registry* registry;
    struct wl_compositor* compositor;
    struct wl_shell_surface* shell_surface;

    struct _egl {
        EGLConfig config;
        EGLContext context;
        EGLDisplay display;
        EGLSurface surface;

        GLuint vShader;
        GLuint fShader;
        GLuint pObject;

        GLuint textureId;
        GLint positionLoc;
        GLint texCoordLoc;
        GLint samplerLoc;
        struct wl_egl_window* window;
    } egl;
    
    struct _org {
        int w;
        int h;
        int bpp;
        int size;
    } info;

    int init;
    int ready;
    int flip;
    uint8_t* data;
    uint16_t* pixels[MAX_FB];
};

#endif

#endif

