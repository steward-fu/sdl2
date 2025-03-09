// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#include "../../SDL_internal.h"

#if SDL_VIDEO_RENDER_MINI

#include <unistd.h>
#include <stdbool.h>

#include "SDL_hints.h"
#include "../SDL_sysrender.h"
#include "../../video/mini/SDL_video_mini.h"

typedef struct Mini_TextureData {
    void *data;
    uint32_t w;
    uint32_t h;
    uint32_t fmt;
    uint32_t pitch;
} Mini_TextureData;

typedef struct {
    SDL_bool is_init;
} Mini_RenderData;

#define MAX_TEXTURE 10

struct MY_TEXTURE {
    int pitch;
    const void *pixels;
    SDL_Texture *texture;
};

extern int FB_W;
extern int FB_H;
extern SDL_Window *vid_win;

static struct MY_TEXTURE mytex[MAX_TEXTURE] = {0};

static int update_texture(void *chk, void *new, const void *pixels, int pitch)
{
    int cc = 0;

    for (cc = 0; cc < MAX_TEXTURE; cc++) {
        if (mytex[cc].texture == chk) {
            mytex[cc].texture = new;
            mytex[cc].pixels = pixels;
            mytex[cc].pitch = pitch;
            return cc;
        }
    }
    return -1;
}

const void* get_pixels(void *chk)
{
    int cc = 0;

    for (cc = 0; cc < MAX_TEXTURE; cc++) {
        if (mytex[cc].texture == chk) {
            return mytex[cc].pixels;
        }
    }
    return NULL;
}

static int get_pitch(void *chk)
{
    int cc = 0;

    for (cc = 0; cc < MAX_TEXTURE; cc++) {
        if (mytex[cc].texture == chk) {
            return mytex[cc].pitch;
        }
    }
    return -1;
}

static void Mini_WindowEvent(SDL_Renderer *renderer, const SDL_WindowEvent *event)
{
    debug("%s\n", __func__);
}

static int Mini_CreateTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    Mini_TextureData *t = (Mini_TextureData *)SDL_calloc(1, sizeof(Mini_TextureData));

    debug("%s\n", __func__);
    if (!t) {
        debug("%s, failed to create texture\n", __func__);
        return SDL_OutOfMemory();
    }

    t->w = texture->w;
    t->h = texture->h;
    t->fmt = texture->format;
    t->pitch = t->w * SDL_BYTESPERPIXEL(t->fmt);
    t->data = SDL_calloc(1, t->h * t->pitch);
    if (!t->data) {
        debug("%s, failed to create texture data\n", __func__);
        SDL_free(t);
        return SDL_OutOfMemory();
    }

    texture->driverdata = t;
    update_texture(NULL, texture, NULL, 0);
    GFX_Clear();
    return 0;
}

static int Mini_LockTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect, void **pixels, int *pitch)
{
    Mini_TextureData *t = (Mini_TextureData *)texture->driverdata;

    debug("%s\n", __func__);
    *pixels = t->data;
    *pitch = t->pitch;
    return 0;
}

static int Mini_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect, const void *pixels, int pitch)
{
    debug("%s\n", __func__);
    update_texture(texture, texture, pixels, pitch);
    return 0;
}

static void Mini_UnlockTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    SDL_Rect rect = {0};
    Mini_TextureData *t = (Mini_TextureData *)texture->driverdata;

    debug("%s\n", __func__);
    rect.x = 0;
    rect.y = 0;
    rect.w = texture->w;
    rect.h = texture->h;
    Mini_UpdateTexture(renderer, texture, &rect, t->data, t->pitch);
}

static void Mini_SetTextureScaleMode(SDL_Renderer *renderer, SDL_Texture *texture, SDL_ScaleMode scaleMode)
{
    debug("%s\n", __func__);
}

static int Mini_SetRenderTarget(SDL_Renderer *renderer, SDL_Texture *texture)
{
    debug("%s\n", __func__);
    return 0;
}

static int Mini_QueueSetViewport(SDL_Renderer *renderer, SDL_RenderCommand *cmd)
{
    debug("%s\n", __func__);
    return 0;
}

static int Mini_QueueDrawPoints(SDL_Renderer *renderer, SDL_RenderCommand *cmd, const SDL_FPoint *points, int count)
{
    debug("%s\n", __func__);
    return 0;
}

static int Mini_QueueGeometry(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture,
    const float *xy, int xy_stride, const SDL_Color *color, int color_stride, const float *uv, int uv_stride,
    int num_vertices, const void *indices, int num_indices, int size_indices,
    float scale_x, float scale_y)
{
    debug("%s\n", __func__);
    return 0;
}

static int Mini_QueueFillRects(SDL_Renderer *renderer, SDL_RenderCommand *cmd, const SDL_FRect *rects, int count)
{
    debug("%s\n", __func__);
    return 0;
}

static int Mini_QueueCopy(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_FRect *dstrect)
{
    int pitch = 0;
    const void *pixels = get_pixels(texture);
    SDL_Rect dst = { 0 };
    SDL_Rect src = {srcrect->x, srcrect->y, srcrect->w, srcrect->h};

    int c0 = FB_W / vid_win->w;
    int c1 = FB_H / vid_win->h;
    float scale = c0 > c1 ? c1 : c0;

    dst.w = dstrect->w * scale;
    dst.h = dstrect->h * scale;
    dst.x = dstrect->x * scale;
    dst.y = dstrect->y * scale;
    dst.x += ((FB_W - (vid_win->w * scale)) / 2);
    dst.y += ((FB_H - (vid_win->h * scale)) / 2);

    debug("%s, org dst:%.2f,%.2f,%.2f,%.2f\n", __func__, dstrect->x, dstrect->y, dstrect->w, dstrect->h);
    debug("%s, new src:%d,%d,%d,%d, dst:%d,%d,%d,%d, scale=%.2f\n", __func__, src.x, src.y, src.w, src.h, dst.x, dst.y, dst.w, dst.h, scale);
    pitch = get_pitch(texture);
    if ((pitch == 0) || (pixels == NULL)) {
        return 0;
    }

    GFX_Copy(pixels, src, dst, pitch, 0, E_MI_GFX_ROTATE_180);
    return 0;
}

static int Mini_QueueCopyEx(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture,
    const SDL_Rect *srcrect, const SDL_FRect *dstrect, const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip)
{
    debug("%s\n", __func__);
    return 0;
}

static int Mini_RunCommandQueue(SDL_Renderer *renderer, SDL_RenderCommand *cmd, void *vertices, size_t vertsize)
{
    debug("%s\n", __func__);
    return 0;
}

static int Mini_RenderReadPixels(SDL_Renderer *renderer, const SDL_Rect *rect, Uint32 pixel_format, void *pixels, int pitch)
{
    debug("%s\n", __func__);
    return SDL_Unsupported();
}

static void Mini_RenderPresent(SDL_Renderer *renderer)
{
    debug("%s\n", __func__);
    GFX_Flip();
}

static void Mini_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    Mini_TextureData *t = (Mini_TextureData *)texture->driverdata;

    debug("%s\n", __func__);
    if (t) {
        update_texture(texture, NULL, NULL, 0);
        if (t->data) {
            SDL_free(t->data);
        }
        SDL_free(t);
        texture->driverdata = NULL;
    }
}

static void Mini_DestroyRenderer(SDL_Renderer *renderer)
{
    Mini_RenderData *data = (Mini_RenderData *)renderer->driverdata;

    debug("%s\n", __func__);
    if (data) {
        if (!data->is_init) {
            return;
        }

        data->is_init = SDL_FALSE;
        SDL_free(data);
    }
    SDL_free(renderer);
}

static int Mini_SetVSync(SDL_Renderer *renderer, const int vsync)
{
    debug("%s\n", __func__);
    return 0;
}

SDL_Renderer *Mini_CreateRenderer(SDL_Window *window, Uint32 flags)
{
    SDL_Renderer *renderer = NULL;
    Mini_RenderData *data = NULL;

    debug("%s\n", __func__);
    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(SDL_Renderer));
    if (!renderer) {
        debug("%s, failed to create render\n", __func__);
        SDL_OutOfMemory();
        return NULL;
    }

    data = (Mini_RenderData *) SDL_calloc(1, sizeof(Mini_RenderData));
    if (!data) {
        debug("%s, failed to create render data\n", __func__);
        Mini_DestroyRenderer(renderer);
        SDL_OutOfMemory();
        return NULL;
    }

    renderer->WindowEvent = Mini_WindowEvent;
    renderer->CreateTexture = Mini_CreateTexture;
    renderer->UpdateTexture = Mini_UpdateTexture;
    renderer->LockTexture = Mini_LockTexture;
    renderer->UnlockTexture = Mini_UnlockTexture;
    renderer->SetTextureScaleMode = Mini_SetTextureScaleMode;
    renderer->SetRenderTarget = Mini_SetRenderTarget;
    renderer->QueueSetViewport = Mini_QueueSetViewport;
    renderer->QueueSetDrawColor = Mini_QueueSetViewport;
    renderer->QueueDrawPoints = Mini_QueueDrawPoints;
    renderer->QueueDrawLines = Mini_QueueDrawPoints;
    renderer->QueueGeometry = Mini_QueueGeometry;
    renderer->QueueFillRects = Mini_QueueFillRects;
    renderer->QueueCopy = Mini_QueueCopy;
    renderer->QueueCopyEx = Mini_QueueCopyEx;
    renderer->RunCommandQueue = Mini_RunCommandQueue;
    renderer->RenderReadPixels = Mini_RenderReadPixels;
    renderer->RenderPresent = Mini_RenderPresent;
    renderer->DestroyTexture = Mini_DestroyTexture;
    renderer->DestroyRenderer = Mini_DestroyRenderer;
    renderer->SetVSync = Mini_SetVSync;
    renderer->info = Mini_RenderDriver.info;
    renderer->info.flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
    renderer->driverdata = data;
    renderer->window = window;

    if(data->is_init != SDL_FALSE) {
        return 0;
    }
    data->is_init = SDL_TRUE;
    return renderer;
}

SDL_RenderDriver Mini_RenderDriver = {
    .CreateRenderer = Mini_CreateRenderer,
    .info = {
        .name = "Miyoo Mini",
        .flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE,
        .num_texture_formats = 2,
        .texture_formats = {
            [0] = SDL_PIXELFORMAT_RGB565,
            [1] = SDL_PIXELFORMAT_ARGB8888,
        },
        .max_texture_width = 640,
        .max_texture_height = 480,
    }
};

#endif

