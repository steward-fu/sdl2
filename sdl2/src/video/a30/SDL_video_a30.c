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

#include "../../SDL_internal.h"
#include "SDL_version.h"
#include "SDL_syswm.h"
#include "SDL_loadso.h"
#include "SDL_events.h"
#include "../SDL_sysvideo.h"
#include "../../events/SDL_events_c.h"

#include "SDL_video_a30.h"
#include "SDL_opengles_a30.h"

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
    int fd = -1;
    struct fb_var_screeninfo vinfo = {0};
    SDL_DisplayData *data = NULL;
    SDL_VideoDisplay display = {0};
    SDL_DisplayMode current_mode = {0};

    data = (SDL_DisplayData *)SDL_calloc(1, sizeof(SDL_DisplayData));
    if (data == NULL) {
        return SDL_OutOfMemory();
    }

    fd = open("/dev/fb0", O_RDWR, 0);
    if (fd < 0) {
        return printf(PREFIX"Failed to open framebuffer device");
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        A30_VideoQuit(_this);
        return printf(PREFIX"Failed to get framebuffer information");
    }
    close(fd);

    data->native_display.width = LCD_W;
    data->native_display.height = LCD_H;

    SDL_zero(current_mode);
    current_mode.w = LCD_W;
    current_mode.h = LCD_H;
    current_mode.refresh_rate = 60;
    current_mode.format = SDL_PIXELFORMAT_ABGR8888;
    current_mode.driverdata = NULL;

    SDL_zero(display);
    display.desktop_mode = current_mode;
    display.current_mode = current_mode;
    display.driverdata = data;
    SDL_AddVideoDisplay(&display, SDL_FALSE);
    return 0;
}

void A30_VideoQuit(_THIS)
{
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
    window->flags |= SDL_WINDOW_OPENGL;

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
        return printf(PREFIX"Failed to create EGL window surface");
    }
    displaydata->native_display.width = LCD_W;
    displaydata->native_display.height = LCD_H;

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

void A30_PumpEvents(_THIS)
{
}

