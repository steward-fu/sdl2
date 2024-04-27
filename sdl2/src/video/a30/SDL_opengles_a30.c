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

#include "SDL_opengles_a30.h"
#include "SDL_video_a30.h"

#if 0
SDL_EGL_CreateContext_impl(A30)
SDL_EGL_SwapWindow_impl(A30)
SDL_EGL_MakeCurrent_impl(A30)
#endif

static EGLDisplay display = 0;
static EGLContext context = 0;
static EGLSurface surface = 0;
static EGLConfig config = 0;

extern int need_screen_rotation_helper;

int A30_GLES_LoadLibrary(_THIS, const char *path)
{
    return SDL_EGL_LoadLibrary(_this, path, EGL_DEFAULT_DISPLAY, 0);
}

void A30_GLES_DefaultProfileConfig(_THIS, int *mask, int *major, int *minor)
{
    *mask = SDL_GL_CONTEXT_PROFILE_ES;
    *major = 2;
    *minor = 0;
    printf(PREFIX"Set OpenGL ES v2.0\n");
}

SDL_GLContext A30_GLES_CreateContext(_THIS, SDL_Window *window)
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
    printf(PREFIX"EGL Display Pointer (%p)\n", display);
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
    printf(PREFIX"EGL Config Pointer (%p)\n", config);

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, (EGLint *)&egl_ctx_attr);
    if (context == EGL_NO_CONTEXT) {
        printf(PREFIX"Failed to create EGL context\n");
        return NULL;
    }

    surface = eglCreateWindowSurface(display, config, 0, (EGLint*)&egl_surf_attr);
    printf(PREFIX"EGL Surface Pointer (%p)\n", surface);
    if (surface == EGL_NO_SURFACE) {
        printf(PREFIX"Failed to create EGL surface\n");
        return NULL;
    }

    eglMakeCurrent(display, surface, surface, context);
    printf(PREFIX"Created EGL Surface successfully\n");
    return context;
}

int A30_GLES_SwapWindow(_THIS, SDL_Window *window)
{
    return eglSwapBuffers(display, surface) == EGL_TRUE ? 0 : -1;
}

int A30_GLES_MakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context)
{
    return 0;
}

