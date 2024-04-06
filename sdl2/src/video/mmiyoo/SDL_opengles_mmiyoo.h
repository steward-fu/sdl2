/*
  Customized version for Miyoo-Mini handheld.
  Only tested under Miyoo-Mini stock OS (original firmware) with Parasyte compatible layer.

  Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>
  Copyright (C) 2022-2022 Steward Fu <steward.fu@gmail.com>

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
#ifndef __SDL_OPENGLES_MMIYOO_H__
#define __SDL_OPENGLES_MMIYOO_H__

#include "../SDL_sysvideo.h"

#ifdef QX1000
#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#else
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

typedef struct SDL_GLDriverData {
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
} SDL_GLDriverData;

void *glGetProcAddress(_THIS, const char *proc);
int glGetConfig(EGLConfig *pconf, int *pformat);
int glLoadLibrary(_THIS, const char *name);
int glSetSwapInterval(_THIS, int interval);
int glSwapWindow(_THIS, SDL_Window *window);
int glMakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context);
int glUpdateBufferSettings(void *cb, void *buf);
void glDeleteContext(_THIS, SDL_GLContext context);
void glUnloadLibrary(_THIS);
SDL_GLContext glCreateContext(_THIS, SDL_Window *window);

#endif

