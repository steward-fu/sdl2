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

#ifndef _SDL_OPENLGES_A30_H__
#define _SDL_OPENGLES_A30_H__

#include "../SDL_sysvideo.h"
#include "../SDL_egl_c.h"

#define A30_GLES_GetAttribute       SDL_EGL_GetAttribute
#define A30_GLES_GetProcAddress     SDL_EGL_GetProcAddress
#define A30_GLES_UnloadLibrary      SDL_EGL_UnloadLibrary
#define A30_GLES_SetSwapInterval    SDL_EGL_SetSwapInterval
#define A30_GLES_GetSwapInterval    SDL_EGL_GetSwapInterval
#define A30_GLES_DeleteContext      SDL_EGL_DeleteContext

typedef struct SDL_GLDriverData {
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
} SDL_GLDriverData;

int A30_GLES_GetConfig(EGLConfig *pconf, int *pformat);
int A30_GLES_LoadLibrary(_THIS, const char *name);
int A30_GLES_SwapWindow(_THIS, SDL_Window *window);
int A30_GLES_MakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context);
void A30_GLES_DefaultProfileConfig(_THIS, int *mask, int *major, int *minor);
SDL_GLContext A30_GLES_CreateContext(_THIS, SDL_Window *window);

#endif

