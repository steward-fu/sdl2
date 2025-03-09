// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_MINI

#include "SDL_video_mini.h"
#include "SDL_gles_mini.h"

static EGLDisplay display = 0;
static EGLContext context = 0;
static EGLSurface surface = 0;
static EGLConfig config = 0;
static void *fb_cb = NULL;

EGLBoolean eglUpdateBufferSettings(EGLDisplay display, EGLSurface surface, void *cb, void *p0, void *p1);

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

    printf(PREFIX"Passing Buffer Settings (cb %p)\n", fb_cb);
    eglMakeCurrent(display, surface, surface, context);
    eglUpdateBufferSettings(display, surface, fb_cb, NULL, NULL);
    printf(PREFIX"Prepared EGL successfully\n");
    return context;
}

int glSetSwapInterval(_THIS, int interval)
{
    return 0;
}

int glUpdateBufferSettings(void *cb)
{
    fb_cb = cb;
    printf(PREFIX"Updated Buffer Settings (cb %p)\n", cb);
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
