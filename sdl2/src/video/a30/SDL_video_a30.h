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

#ifndef __SDL_VIDEO_A30_H__
#define __SDL_VIDEO_A30_H__

#include "../../SDL_internal.h"
#include "../SDL_sysvideo.h"
#include "SDL_egl.h"

#include <EGL/egl.h>
#include <linux/vt.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stropts.h>
#include <unistd.h>
#include <stdlib.h>

#define PREFIX  "[SDL2] "
#define LCD_W   640
#define LCD_H   480
#define REAL_W  LCD_H
#define REAL_H  LCD_W

typedef struct SDL_DisplayData {
    struct fbdev_window native_display;
} SDL_DisplayData;

typedef struct SDL_WindowData {
    EGLSurface egl_surface;
} SDL_WindowData;

int A30_VideoInit(_THIS);
int A30_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode);
int A30_CreateWindow(_THIS, SDL_Window * window);
void A30_VideoQuit(_THIS);
void A30_GetDisplayModes(_THIS, SDL_VideoDisplay * display);
void A30_SetWindowTitle(_THIS, SDL_Window * window);
void A30_SetWindowPosition(_THIS, SDL_Window * window);
void A30_SetWindowSize(_THIS, SDL_Window * window);
void A30_ShowWindow(_THIS, SDL_Window * window);
void A30_HideWindow(_THIS, SDL_Window * window);
void A30_DestroyWindow(_THIS, SDL_Window * window);
void A30_PumpEvents(_THIS);
SDL_bool A30_GetWindowWMInfo(_THIS, SDL_Window * window, struct SDL_SysWMinfo *info);

#endif

