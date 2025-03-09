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
    debug("%s\n", __func__);
    return 0;
}

void *glGetProcAddress(_THIS, const char *proc)
{
    debug("%s\n", __func__);
    return eglGetProcAddress(proc);
}

void glUnloadLibrary(_THIS)
{
    debug("%s\n", __func__);
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
        .client_version = {
            EGL_CONTEXT_CLIENT_VERSION,
            2
        },
        .none = EGL_NONE
    };

    struct {
        EGLint render_buffer[2];
        EGLint none;
    } egl_surf_attr = {
        .render_buffer = {
            EGL_RENDER_BUFFER,
            EGL_BACK_BUFFER
        },
        .none = EGL_NONE
    };

    debug("%s, display=%p\n", __func__, display);
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, &majorVersion, &minorVersion);
    eglGetConfigs(display, NULL, 0, &numConfigs);
    cfgs = SDL_malloc(numConfigs * sizeof(EGLConfig));
    if (cfgs == NULL) {
        debug("%s, failed to allocate memory for EGL config\n", __func__);
        return NULL;
    }

    rc = eglGetConfigs(display, cfgs, numConfigs, &numConfigs);
    if (rc != EGL_TRUE) {
        SDL_free(cfgs);
        debug("%s, failed to get EGL config\n", __func__);
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
    debug("%s, EGL Config=%p\n", __func__, config);

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, (EGLint *)&egl_ctx_attr);
    if (context == EGL_NO_CONTEXT) {
        debug("%s, failed to create EGL context\n", __func__);
        return NULL;
    }

    surface = eglCreateWindowSurface(display, config, 0, (EGLint*)&egl_surf_attr);
    debug("%s, EGL Surface=%p\n", __func__, surface);
    if (surface == EGL_NO_SURFACE) {
        debug("%s, failed to create EGL surface\n", __func__);
        return NULL;
    }

    debug("%s, callback=%p\n", __func__, fb_cb);
    eglMakeCurrent(display, surface, surface, context);
    eglUpdateBufferSettings(display, surface, fb_cb, NULL, NULL);
    debug("%s, initialized EGL successfully\n", __func__);
    return context;
}

int glSetSwapInterval(_THIS, int interval)
{
    debug("%s\n", __func__);
    return 0;
}

int glUpdateBufferSettings(void *cb)
{
    fb_cb = cb;
    debug("%s, callback=%p\n", __func__, cb);
    return 0;
}

int glSwapWindow(_THIS, SDL_Window *window)
{
    debug("%s\n", __func__);
    return eglSwapBuffers(display, surface) == EGL_TRUE ? 0 : -1;
}

int glMakeCurrent(_THIS, SDL_Window *window, SDL_GLContext context)
{
    debug("%s\n", __func__);
    return 0;
}

void glDeleteContext(_THIS, SDL_GLContext context)
{
    debug("%s\n", __func__);
}

#endif
