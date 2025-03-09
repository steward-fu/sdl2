// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#ifndef __SDL_GLES_MINI_H__
#define __SDL_GLES_MINI_H__

#include "../SDL_sysvideo.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

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
int glUpdateBufferSettings(void *cb);
void glDeleteContext(_THIS, SDL_GLContext context);
void glUnloadLibrary(_THIS);
SDL_GLContext glCreateContext(_THIS, SDL_Window *window);

#endif

