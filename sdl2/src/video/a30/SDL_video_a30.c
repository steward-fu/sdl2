/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "../../SDL_internal.h"
#include "SDL_version.h"
#include "SDL_syswm.h"
#include "SDL_loadso.h"
#include "SDL_events.h"
#include "../SDL_sysvideo.h"
#include "../../events/SDL_events_c.h"

#include "SDL_event_a30.h"
#include "SDL_video_a30.h"
#include "SDL_opengles_a30.h"

static int running = 0;
static pthread_t thread = 0;

int fb_flip = 0;
int fb_dev = -1;
uint32_t *gl_mem = NULL;
uint32_t *fb_mem = NULL;
struct fb_var_screeninfo vinfo = {0};
int need_screen_rotation_helper = 0;
A30_VideoInfo vid = {0};

extern EGLConfig eglConfig;
extern EGLDisplay eglDisplay;
extern EGLContext eglContext;
extern EGLSurface eglSurface;

static uint32_t LUT_RT[640 * 480] = {0};
static uint32_t LUT_GL[640 * 480] = {0};

void* video_handler(void *data)
{
    static int frame = 0;

    int x = 0;
    int y = 0;
    int idx = 0;
    int ret = 0;
    uint32_t v = 0;
    uint32_t *src = NULL;
    uint32_t *dst = NULL;

    v = 0;
    for (y = 0; y < 640; y++) {
        for (x = 0; x < 480; x++) {
            LUT_RT[v++] = ((639 - y) * 480) + x;
        }
    }

    v = 0;
    for (y = 0; y < 480; y++) {
        for (x = 0; x < 640; x++) {
            LUT_GL[v++] = ((639 - x) * 480) + (479 - y);
        }
    }

    while (running) {
        if (fb_flip) {
            idx = 0;
            fb_flip = 0;
            src = gl_mem;
            dst = fb_mem + (640 * 480 * (frame % 2));

            if (need_screen_rotation_helper) {
                for (y = 0; y < 640; y++) {
                    for (x = 0; x < 480; x++) {
                        v = *src++;
                        dst[LUT_RT[idx++]] = 0xff000000 | ((v & 0xff) << 16) | (v & 0xff00) | ((v & 0xff0000) >> 16);
                    }
                }
            }
            else {
                for (y = 0; y < 480; y++) {
                    for (x = 0; x < 640; x++) {
                        v = *src++;
                        dst[LUT_GL[idx++]] = 0xff000000 | ((v & 0xff) << 16) | (v & 0xff00) | ((v & 0xff0000) >> 16);
                    }
                }
            }
            vinfo.yoffset = (frame % 2) * vinfo.yres;
            ioctl(fb_dev, FBIOPAN_DISPLAY, &vinfo);
            ioctl(fb_dev, FBIO_WAITFORVSYNC, &ret);
            frame += 1;
        }
        else {
            usleep(10);
        }
    }
    return NULL;
}

static void A30_Destroy(SDL_VideoDevice *device)
{
    if (device->driverdata != NULL) {
        SDL_free(device->driverdata);
        device->driverdata = NULL;
    }
}

static SDL_VideoDevice* A30_CreateDevice(int devindex)
{
    SDL_VideoDevice *device = NULL;

    device = (SDL_VideoDevice *)SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (device == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }

    device->driverdata = NULL;
    device->num_displays = 0;
    device->free = A30_Destroy;
    device->VideoInit = A30_VideoInit;
    device->VideoQuit = A30_VideoQuit;
    device->GetDisplayModes = A30_GetDisplayModes;
    device->SetDisplayMode = A30_SetDisplayMode;
    device->CreateSDLWindow = A30_CreateWindow;
    device->SetWindowTitle = A30_SetWindowTitle;
    device->SetWindowPosition = A30_SetWindowPosition;
    device->SetWindowSize = A30_SetWindowSize;
    device->ShowWindow = A30_ShowWindow;
    device->HideWindow = A30_HideWindow;
    device->DestroyWindow = A30_DestroyWindow;
    device->GetWindowWMInfo = A30_GetWindowWMInfo;

    device->GL_LoadLibrary = A30_GLES_LoadLibrary;
    device->GL_GetProcAddress = A30_GLES_GetProcAddress;
    device->GL_UnloadLibrary = A30_GLES_UnloadLibrary;
    device->GL_CreateContext = A30_GLES_CreateContext;
    device->GL_MakeCurrent = A30_GLES_MakeCurrent;
    device->GL_SetSwapInterval = A30_GLES_SetSwapInterval;
    device->GL_GetSwapInterval = A30_GLES_GetSwapInterval;
    device->GL_SwapWindow = A30_GLES_SwapWindow;
    device->GL_DeleteContext = A30_GLES_DeleteContext;
    device->GL_DefaultProfileConfig = A30_GLES_DefaultProfileConfig;
    device->PumpEvents = A30_PumpEvents;
    return device;
}

VideoBootStrap A30_bootstrap = { "a30", "Miyoo A30 Video Driver", A30_CreateDevice };

int A30_VideoInit(_THIS)
{
    SDL_DisplayData *data = NULL;
    SDL_VideoDisplay display = {0};
    SDL_DisplayMode current_mode = {0};

    printf(PREFIX"%s\n", __func__);
    data = (SDL_DisplayData *)SDL_calloc(1, sizeof(SDL_DisplayData));
    if (data == NULL) {
        return SDL_OutOfMemory();
    }

    fb_dev = open("/dev/fb0", O_RDWR, 0);
    if (fb_dev < 0) {
        printf(PREFIX"Failed to open framebuffer device\n");
        return -1;
    }

    if (ioctl(fb_dev, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        A30_VideoQuit(_this);
        printf(PREFIX"Failed to get framebuffer information\n");
        return -1;
    }

    fb_mem = (uint32_t *)mmap(NULL, FB_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fb_dev, 0);
    if (fb_mem == (void *)-1) {
        close(fb_dev);
        fb_dev = -1;
        printf(PREFIX"Failed to mmap /dev/fb0\n");
        return -1;
    }
    printf(PREFIX"fb_mem %p\n", fb_mem);
    memset(fb_mem, 0 , FB_SIZE);

    gl_mem = malloc(GL_SIZE);
    if (gl_mem == NULL) {
        printf(PREFIX"Failed to allocate glmem\n");
        return -1;
    }
    printf(PREFIX"gl_mem %p\n", gl_mem);
    memset(gl_mem, 0 , GL_SIZE);

    vinfo.yres_virtual = vinfo.yres * 2;
    ioctl(fb_dev, FBIOPUT_VSCREENINFO, &vinfo);

    data->native_display.width = LCD_W;
    data->native_display.height = LCD_H;

    SDL_zero(current_mode);
    current_mode.w = 640; //data->native_display.width;
    current_mode.h = 480; //data->native_display.height;
    current_mode.refresh_rate = 60;
    current_mode.format = SDL_PIXELFORMAT_RGBA8888;
    current_mode.driverdata = NULL;

    SDL_zero(display);
    display.desktop_mode = current_mode;
    display.current_mode = current_mode;
    display.driverdata = data;
    SDL_AddVideoDisplay(&display, SDL_FALSE);

    A30_EventInit();

    running = 1;
    pthread_create(&thread, NULL, video_handler, NULL);
    return 0;
}

void A30_VideoQuit(_THIS)
{
    printf(PREFIX"%s\n", __func__);

    running = 0;
    pthread_join(thread, NULL);

    if (fb_mem) {
        munmap(fb_mem, FB_SIZE);
        fb_mem = NULL;
    }

    if (fb_dev > 0) {
        close(fb_dev);
        fb_dev = -1;
    }

    if (gl_mem) {
        free(gl_mem);
        gl_mem = NULL;
    }
}

void A30_GetDisplayModes(_THIS, SDL_VideoDisplay *display)
{
    SDL_AddDisplayMode(display, &display->current_mode);
}

int A30_SetDisplayMode(_THIS, SDL_VideoDisplay *display, SDL_DisplayMode *mode)
{
    return 0;
}

int A30_CreateWindow(_THIS, SDL_Window *window)
{
    SDL_WindowData *windowdata = NULL;
    SDL_DisplayData *displaydata = NULL;

    displaydata = SDL_GetDisplayDriverData(0);
    windowdata = (SDL_WindowData *)SDL_calloc(1, sizeof(SDL_WindowData));
    if (windowdata == NULL) {
        return SDL_OutOfMemory();
    }
    window->w = displaydata->native_display.width;
    window->h = displaydata->native_display.height;

    need_screen_rotation_helper = 0;
    if (!(window->flags & SDL_WINDOW_OPENGL)) {
        need_screen_rotation_helper = 1;
        window->flags |= SDL_WINDOW_OPENGL;
    }
    printf(PREFIX"%s Screen Rotation Helper\n", need_screen_rotation_helper ? "Enabled" : "Disabled");

    if (!_this->egl_data) {
        if (SDL_GL_LoadLibrary(NULL) < 0) {
            return -1;
        }
    }

    displaydata->native_display.width = REAL_W;
    displaydata->native_display.height = REAL_H;
    windowdata->egl_surface = SDL_EGL_CreateSurface(_this, (NativeWindowType)&displaydata->native_display);
    if (windowdata->egl_surface == EGL_NO_SURFACE) {
        A30_VideoQuit(_this);
        return printf(PREFIX"Failed to create EGL window surface\n");
    }
    displaydata->native_display.width = LCD_W;
    displaydata->native_display.height = LCD_H;

    vid.window = window;
    window->driverdata = windowdata;
    SDL_SetMouseFocus(window);
    SDL_SetKeyboardFocus(window);
    return 0;
}

void A30_DestroyWindow(_THIS, SDL_Window *window)
{
    SDL_WindowData *data = NULL;

    data = window->driverdata;
    if (data) {
        if (data->egl_surface != EGL_NO_SURFACE) {
            SDL_EGL_DestroySurface(_this, data->egl_surface);
            data->egl_surface = EGL_NO_SURFACE;
        }
        SDL_free(data);
    }
    window->driverdata = NULL;
}

void A30_SetWindowTitle(_THIS, SDL_Window *window)
{
}

void A30_SetWindowPosition(_THIS, SDL_Window *window)
{
}

void A30_SetWindowSize(_THIS, SDL_Window *window)
{
}

void A30_ShowWindow(_THIS, SDL_Window *window)
{
}

void A30_HideWindow(_THIS, SDL_Window *window)
{
}

SDL_bool A30_GetWindowWMInfo(_THIS, SDL_Window *window, struct SDL_SysWMinfo *info)
{
    return SDL_TRUE;
}

