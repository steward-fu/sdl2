// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#include "../../SDL_internal.h"

#if SDL_VIDEO_RENDER_XT897

#include <unistd.h>
#include <stdbool.h>

#include "SDL_hints.h"
#include "../SDL_sysrender.h"

#include "../../video/xt897/SDL_video_xt897.h"
#include "../../video/xt897/SDL_event_xt897.h"

typedef struct XT897_Texture {
    uint32_t w;
    uint32_t h;
    uint32_t fmt;
    uint32_t pitch;
} XT897_Texture;

typedef struct {
    SDL_bool is_init;
} XT897_Renderer;

extern struct _wayland wl;

static void XT897_WindowEvent(SDL_Renderer *renderer, const SDL_WindowEvent *event)
{
    debug("%s\n", __func__);
}

static int XT897_CreateTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    XT897_Texture *t = (XT897_Texture *)SDL_calloc(1, sizeof(XT897_Texture));

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

static int XT897_LockTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect, void **pixels, int *pitch)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT897_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect, const void *pixels, int pitch)
{
    debug("%s\n", __func__);
    memcpy(wl.pixels[wl.flip], pixels, rect->h * pitch);
    return 0;
}

static void XT897_UnlockTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    debug("%s\n", __func__);
}

static void XT897_SetTextureScaleMode(SDL_Renderer *renderer, SDL_Texture *texture, SDL_ScaleMode scaleMode)
{
    debug("%s\n", __func__);
}

static int XT897_SetRenderTarget(SDL_Renderer *renderer, SDL_Texture *texture)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT897_QueueSetViewport(SDL_Renderer *renderer, SDL_RenderCommand *cmd)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT897_QueueDrawPoints(SDL_Renderer *renderer, SDL_RenderCommand *cmd, const SDL_FPoint *points, int count)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT897_QueueGeometry(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture,
                                const float *xy, int xy_stride, const SDL_Color *color, int color_stride, const float *uv, int uv_stride,
                                int num_vertices, const void *indices, int num_indices, int size_indices,
                                float scale_x, float scale_y)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT897_QueueFillRects(SDL_Renderer *renderer, SDL_RenderCommand *cmd, const SDL_FRect *rects, int count)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT897_QueueCopy(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_FRect *dstrect)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT897_QueueCopyEx(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture,
    const SDL_Rect *srcrect, const SDL_FRect *dstrect, const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT897_RunCommandQueue(SDL_Renderer *renderer, SDL_RenderCommand *cmd, void *vertices, size_t vertsize)
{
    debug("%s\n", __func__);
    return 0;
}

static int XT897_RenderReadPixels(SDL_Renderer *renderer, const SDL_Rect *rect, Uint32 pixel_format, void *pixels, int pitch)
{
    debug("%s\n", __func__);
    return SDL_Unsupported();
}

static void XT897_RenderPresent(SDL_Renderer *renderer)
{
    debug("%s\n", __func__);
    wl.flip ^= 1;
}

static void XT897_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    XT897_Texture *t = (XT897_Texture*)texture->driverdata;

    debug("%s\n", __func__);
    if (t) {
        SDL_free(t);
        texture->driverdata = NULL;
    }
}

static void XT897_DestroyRenderer(SDL_Renderer *renderer)
{
    XT897_Renderer *t = (XT897_Renderer *)renderer->driverdata;

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

static int XT897_SetVSync(SDL_Renderer *renderer, const int vsync)
{
    debug("%s\n", __func__);
    return 0;
}

SDL_Renderer *XT897_CreateRenderer(SDL_Window *window, Uint32 flags)
{
    SDL_Renderer *renderer = NULL;
    XT897_Renderer *data = NULL;

    debug("%s\n", __func__);
    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(SDL_Renderer));
    if (!renderer) {
        SDL_OutOfMemory();
        return NULL;
    }

    data = (XT897_Renderer *) SDL_calloc(1, sizeof(XT897_Renderer));
    if (!data) {
        XT897_DestroyRenderer(renderer);
        SDL_OutOfMemory();
        return NULL;
    }

    renderer->WindowEvent = XT897_WindowEvent;
    renderer->CreateTexture = XT897_CreateTexture;
    renderer->UpdateTexture = XT897_UpdateTexture;
    renderer->LockTexture = XT897_LockTexture;
    renderer->UnlockTexture = XT897_UnlockTexture;
    renderer->SetTextureScaleMode = XT897_SetTextureScaleMode;
    renderer->SetRenderTarget = XT897_SetRenderTarget;
    renderer->QueueSetViewport = XT897_QueueSetViewport;
    renderer->QueueSetDrawColor = XT897_QueueSetViewport;
    renderer->QueueDrawPoints = XT897_QueueDrawPoints;
    renderer->QueueDrawLines = XT897_QueueDrawPoints;
    renderer->QueueGeometry = XT897_QueueGeometry;
    renderer->QueueFillRects = XT897_QueueFillRects;
    renderer->QueueCopy = XT897_QueueCopy;
    renderer->QueueCopyEx = XT897_QueueCopyEx;
    renderer->RunCommandQueue = XT897_RunCommandQueue;
    renderer->RenderReadPixels = XT897_RenderReadPixels;
    renderer->RenderPresent = XT897_RenderPresent;
    renderer->DestroyTexture = XT897_DestroyTexture;
    renderer->DestroyRenderer = XT897_DestroyRenderer;
    renderer->SetVSync = XT897_SetVSync;
    renderer->info = XT897_RenderDriver.info;
    renderer->info.flags = (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    renderer->driverdata = data;
    renderer->window = window;

    if (data->is_init != SDL_FALSE) {
        return 0;
    }
    data->is_init = SDL_TRUE;
    return renderer;
}

SDL_RenderDriver XT897_RenderDriver = {
    .CreateRenderer = XT897_CreateRenderer,
    .info = {
        .name = "XT897",
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

