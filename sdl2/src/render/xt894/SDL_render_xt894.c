// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#include "../../SDL_internal.h"

#if SDL_VIDEO_RENDER_XT894

#include <unistd.h>
#include <stdbool.h>

#include "SDL_hints.h"
#include "../SDL_sysrender.h"

#include "../../video/xt894/SDL_video_xt894.h"
#include "../../video/xt894/SDL_event_xt894.h"

typedef struct XT894_Texture {
    uint32_t w;
    uint32_t h;
    uint32_t fmt;
    uint32_t pitch;
} XT894_Texture;

typedef struct {
    SDL_bool is_init;
} XT894_Renderer;

extern struct _wayland wl;

static void XT894_WindowEvent(SDL_Renderer *renderer, const SDL_WindowEvent *event)
{
    debug("%s\n", __func__);
}

static int XT894_CreateTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    XT894_Texture *t = (XT894_Texture *)SDL_calloc(1, sizeof(XT894_Texture));

    debug("%s\n", __func__);
    if (!t) {
        debug("Failed to create texture\n");
        return SDL_OutOfMemory();
    }

    t->w = texture->w;
    t->h = texture->h;
    t->fmt = texture->format;
    t->pitch = t->w * SDL_BYTESPERPIXEL(t->fmt);
    texture->driverdata = t;
    return 0;
}

static int XT894_LockTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect, void **pixels, int *pitch)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT894_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect, const void *pixels, int pitch)
{
    debug("%s\n", __func__);
    memcpy(wl.pixels[wl.flip], pixels, rect->h * pitch);
    return 0;
}

static void XT894_UnlockTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    debug("%s\n", __func__);
}

static void XT894_SetTextureScaleMode(SDL_Renderer *renderer, SDL_Texture *texture, SDL_ScaleMode scaleMode)
{
    debug("%s\n", __func__);
}

static int XT894_SetRenderTarget(SDL_Renderer *renderer, SDL_Texture *texture)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT894_QueueSetViewport(SDL_Renderer *renderer, SDL_RenderCommand *cmd)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT894_QueueDrawPoints(SDL_Renderer *renderer, SDL_RenderCommand *cmd, const SDL_FPoint *points, int count)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT894_QueueGeometry(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture,
                                const float *xy, int xy_stride, const SDL_Color *color, int color_stride, const float *uv, int uv_stride,
                                int num_vertices, const void *indices, int num_indices, int size_indices,
                                float scale_x, float scale_y)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT894_QueueFillRects(SDL_Renderer *renderer, SDL_RenderCommand *cmd, const SDL_FRect *rects, int count)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT894_QueueCopy(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_FRect *dstrect)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT894_QueueCopyEx(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture,
    const SDL_Rect *srcrect, const SDL_FRect *dstrect, const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT894_RunCommandQueue(SDL_Renderer *renderer, SDL_RenderCommand *cmd, void *vertices, size_t vertsize)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT894_RenderReadPixels(SDL_Renderer *renderer, const SDL_Rect *rect, Uint32 pixel_format, void *pixels, int pitch)
{
    debug("%s\n", __func__);
    return SDL_Unsupported();
}

static void XT894_RenderPresent(SDL_Renderer *renderer)
{
    debug("%s\n", __func__);
    wl.flip ^= 1;
}

static void XT894_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    XT894_Texture *t = (XT894_Texture*)texture->driverdata;

    debug("%s\n", __func__);
    if (t) {
        SDL_free(t);
        texture->driverdata = NULL;
    }
}

static void XT894_DestroyRenderer(SDL_Renderer *renderer)
{
    XT894_Renderer *t = (XT894_Renderer *)renderer->driverdata;

    debug("%s\n", __func__);
    if (t) {
        if (!t->is_init) {
            return;
        }

        t->is_init = SDL_FALSE;
        SDL_free(t);
    }
    SDL_free(renderer);
}

static int XT894_SetVSync(SDL_Renderer *renderer, const int vsync)
{
    debug("%s\n", __func__);
    return 0;
}

SDL_Renderer *XT894_CreateRenderer(SDL_Window *window, Uint32 flags)
{
    SDL_Renderer *renderer = NULL;
    XT894_Renderer *data = NULL;

    debug("%s\n", __func__);
    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(SDL_Renderer));
    if (!renderer) {
        SDL_OutOfMemory();
        return NULL;
    }

    data = (XT894_Renderer *) SDL_calloc(1, sizeof(XT894_Renderer));
    if (!data) {
        XT894_DestroyRenderer(renderer);
        SDL_OutOfMemory();
        return NULL;
    }

    renderer->WindowEvent = XT894_WindowEvent;
    renderer->CreateTexture = XT894_CreateTexture;
    renderer->UpdateTexture = XT894_UpdateTexture;
    renderer->LockTexture = XT894_LockTexture;
    renderer->UnlockTexture = XT894_UnlockTexture;
    renderer->SetTextureScaleMode = XT894_SetTextureScaleMode;
    renderer->SetRenderTarget = XT894_SetRenderTarget;
    renderer->QueueSetViewport = XT894_QueueSetViewport;
    renderer->QueueSetDrawColor = XT894_QueueSetViewport;
    renderer->QueueDrawPoints = XT894_QueueDrawPoints;
    renderer->QueueDrawLines = XT894_QueueDrawPoints;
    renderer->QueueGeometry = XT894_QueueGeometry;
    renderer->QueueFillRects = XT894_QueueFillRects;
    renderer->QueueCopy = XT894_QueueCopy;
    renderer->QueueCopyEx = XT894_QueueCopyEx;
    renderer->RunCommandQueue = XT894_RunCommandQueue;
    renderer->RenderReadPixels = XT894_RenderReadPixels;
    renderer->RenderPresent = XT894_RenderPresent;
    renderer->DestroyTexture = XT894_DestroyTexture;
    renderer->DestroyRenderer = XT894_DestroyRenderer;
    renderer->SetVSync = XT894_SetVSync;
    renderer->info = XT894_RenderDriver.info;
    renderer->info.flags = (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    renderer->driverdata = data;
    renderer->window = window;

    if (data->is_init != SDL_FALSE) {
        return 0;
    }
    data->is_init = SDL_TRUE;
    return renderer;
}

SDL_RenderDriver XT894_RenderDriver = {
    .CreateRenderer = XT894_CreateRenderer,
    .info = {
        .name = "XT894",
        .flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE,
        .num_texture_formats = 2,
        .texture_formats = {
            [0] = SDL_PIXELFORMAT_RGB565, [2] = SDL_PIXELFORMAT_ARGB8888,
        },
        .max_texture_width = 960,
        .max_texture_height = 540,
    }
};

#endif

