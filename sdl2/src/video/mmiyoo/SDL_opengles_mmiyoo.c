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
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_MMIYOO

#include "SDL_video_mmiyoo.h"
#include "SDL_opengles_mmiyoo.h"

static EGLDisplay display = 0;
static EGLContext context = 0;
static EGLSurface surface = 0;
static EGLConfig config = 0;
static void *fb_cb = NULL;
static void *fb_flip = NULL;
static unsigned long fb_vaddr[2] = {0};

EGLBoolean eglUpdateBufferSettings(EGLDisplay display, EGLSurface surface, void *pFunc, void *fb_idx, void *fb_vaddr);

int glLoadLibrary(_THIS, const char *name)
{
    return 0;
}

void *glGetProcAddress(_THIS, const char *proc)
{
    return eglGetProcAddress(proc);
}

void glUnloadLibrary(_THIS)
{
    eglTerminate(_this->gl_data->display);
}

SDL_GLContext glCreateContext(_THIS, SDL_Window *window)
{
    EGLint i = 0;
    EGLint val = 0;
    EGLBoolean rc = 0;
    EGLConfig *cfgs = NULL;
    EGLint numConfigs = 0;
    EGLint majorVersion = 0;
    EGLint minorVersion = 0;
    struct {
        EGLint client_version[2];
        EGLint none;
    } egl_ctx_attr = {
        .client_version = { EGL_CONTEXT_CLIENT_VERSION, 2 },
        .none = EGL_NONE
    };

    struct {
        EGLint render_buffer[2];
        EGLint none;
    } egl_surf_attr = {
        .render_buffer = { EGL_RENDER_BUFFER, EGL_BACK_BUFFER },
        .none = EGL_NONE
    };

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    printf(PREFIX"EGL Display %p\n", display);
    eglInitialize(display, &majorVersion, &minorVersion);
    eglGetConfigs(display, NULL, 0, &numConfigs);
    cfgs = SDL_malloc(numConfigs * sizeof(EGLConfig));
    if (cfgs == NULL) {
        printf(PREFIX"Failed to allocate memory for EGL config\n");
        return NULL;
    }

    rc = eglGetConfigs(display, cfgs, numConfigs, &numConfigs);
    if (rc != EGL_TRUE) {
        SDL_free(cfgs);
        printf(PREFIX"Failed to get EGL config\n");
        return NULL;
    }

    for (i = 0; i < numConfigs; i++) {
        eglGetConfigAttrib(display, cfgs[i], EGL_SURFACE_TYPE, &val);
        if (!(val & EGL_WINDOW_BIT)) {
            continue;
        }

        eglGetConfigAttrib(display, cfgs[i], EGL_RENDERABLE_TYPE, &val);
        if (!(val & EGL_OPENGL_ES2_BIT)) {
            continue;
        }

        eglGetConfigAttrib(display, cfgs[i], EGL_DEPTH_SIZE, &val);
        if (val == 0) {
            continue;
        }

        config = cfgs[i];
        break;
    }
    SDL_free(cfgs);
    printf(PREFIX"EGL Config %p\n", config);

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, (EGLint *)&egl_ctx_attr);
    if (context == EGL_NO_CONTEXT) {
        printf(PREFIX"Failed to create EGL context\n");
        return NULL;
    }

    surface = eglCreateWindowSurface(display, config, 0, (EGLint*)&egl_surf_attr);
    printf(PREFIX"EGL Surface %p\n", surface);
    if (surface == EGL_NO_SURFACE) {
        printf(PREFIX"Failed to create EGL surface\n");
        return NULL;
    }

    printf(PREFIX"Passing Buffer Settings (CB %p, vAddr0 0x%lx, vAddr1 0x%lx)\n", fb_cb, fb_vaddr[0], fb_vaddr[1]);
    eglMakeCurrent(display, surface, surface, context);
    eglUpdateBufferSettings(display, surface, fb_cb, fb_flip, fb_vaddr);
    printf(PREFIX"Prepared EGL successfully\n");
    return context;
}

int glSetSwapInterval(_THIS, int interval)
{
    return 0;
}

int glUpdateBufferSettings(void *cb, void *flip, void *v0, void *v1)
{
    fb_cb = cb;
    fb_flip = flip;
    fb_vaddr[0] = (unsigned long)v0;
    fb_vaddr[1] = (unsigned long)v1;
    printf(PREFIX"Updated Buffer Settings (CB %p, vAddr0 %p, vAddr1 %p)\n", cb, v0, v1);
    return 0;
}

int glSwapWindow(_THIS, SDL_Window *window)
{
    return eglSwapBuffers(display, surface) == EGL_TRUE ? 0 : -1;
}

int glMakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context)
{
    return 0;
}

void glDeleteContext(_THIS, SDL_GLContext context)
{
}

#endif
