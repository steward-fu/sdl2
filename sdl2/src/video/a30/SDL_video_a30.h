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
#include <EGL/fbdev_window.h>
#include <GLES2/gl2.h>

#include <linux/vt.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stropts.h>
#include <unistd.h>
#include <stdlib.h>

#define PREFIX              "[SDL] "
#define LCD_W               640
#define LCD_H               480
#define CCU_BASE            0x01c20000
#define INIT_CPU_CORE       2
#define INIT_CPU_CLOCK      1500
#define DEINIT_CPU_CORE     2
#define DEINIT_CPU_CLOCK    648

#define USE_MYJOY           0
#define MYJOY_MODE_KEYPAD   0
#define MYJOY_MODE_MOUSE    1

enum _TEX_TYPE {
    TEX_SCR = 0,
    TEX_MAX
};

struct _video {
    SDL_Window *win;
    EGLConfig eglConfig;
    EGLDisplay eglDisplay;
    EGLContext eglContext;
    EGLSurface eglSurface;
    GLuint vShader;
    GLuint fShader;
    GLuint pObject;
    GLuint texID[TEX_MAX];
    GLint posLoc;
    GLint texLoc;
    GLint samLoc;
    GLint alphaLoc;

    int mem_fd;
    int fb_flip;
    void *fb_mem[2];
    uint8_t* ccu_mem;
    uint8_t* dac_mem;
    uint32_t *vol_ptr;
    uint32_t *cpu_ptr;
};

struct _cpu_clock {
    int clk;
    uint32_t reg;
};

#endif

