// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#include "../../SDL_internal.h"

#if SDL_VIDEO_RENDER_MINI

#include <unistd.h>
#include <stdbool.h>

#include "SDL_hints.h"
#include "../SDL_sysrender.h"
#include "../../video/mini/SDL_video_mini.h"
#include "../../video/mini/SDL_event_mini.h"

typedef struct Mini_TextureData {
    void *data;
    unsigned int size;
    unsigned int width;
    unsigned int height;
    unsigned int bits;
    unsigned int format;
    unsigned int pitch;
} Mini_TextureData;

typedef struct {
    SDL_Texture *boundTarget;
    SDL_bool initialized;
    unsigned int bpp;
    SDL_bool vsync;
} Mini_RenderData;

#define MAX_TEXTURE 10

struct _NDS_TEXTURE {
    int pitch;
    const void *pixels;
    SDL_Texture *texture;
};

extern GFX gfx;
extern Mini_EventInfo evt;
extern SDL_Surface *fps_info;
extern int FB_W;
extern int FB_H;
extern int FB_SIZE;
extern int TMP_SIZE;
extern int show_fps;
extern int down_scale;

int need_reload_bg = 0;
static int threading_mode = 0;
static struct _NDS_TEXTURE ntex[MAX_TEXTURE] = {0};

static int update_texture(void *chk, void *new, const void *pixels, int pitch)
{
    int cc = 0;

    for (cc=0; cc<MAX_TEXTURE; cc++) {
        if (ntex[cc].texture == chk) {
            ntex[cc].texture = new;
            ntex[cc].pixels = pixels;
            ntex[cc].pitch = pitch;
            return cc;
        }
    }
    return -1;
}

const void* get_pixels(void *chk)
{
    int cc = 0;

    for (cc=0; cc<MAX_TEXTURE; cc++) {
        if (ntex[cc].texture == chk) {
            return ntex[cc].pixels;
        }
    }
    return NULL;
}

static int get_pitch(void *chk)
{
    int cc = 0;

    for (cc=0; cc<MAX_TEXTURE; cc++) {
        if (ntex[cc].texture == chk) {
            return ntex[cc].pitch;
        }
    }
    return -1;
}

static void Mini_WindowEvent(SDL_Renderer *renderer, const SDL_WindowEvent *event)
{
}

static int Mini_CreateTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    Mini_TextureData *mini_texture = (Mini_TextureData *)SDL_calloc(1, sizeof(*mini_texture));

    if(!mini_texture) {
        printf(PREFIX"Failed to create texture\n");
        return SDL_OutOfMemory();
    }

    mini_texture->width = texture->w;
    mini_texture->height = texture->h;
    mini_texture->format = texture->format;

    switch(mini_texture->format) {
    case SDL_PIXELFORMAT_RGB565:
        mini_texture->bits = 16;
        break;
    case SDL_PIXELFORMAT_ARGB8888:
        mini_texture->bits = 32;
        break;
    default:
        return -1;
    }

    mini_texture->pitch = mini_texture->width * SDL_BYTESPERPIXEL(texture->format);
    mini_texture->size = mini_texture->height * mini_texture->pitch;
    mini_texture->data = SDL_calloc(1, mini_texture->size);

    if(!mini_texture->data) {
        printf(PREFIX"Failed to create texture data\n");
        SDL_free(mini_texture);
        return SDL_OutOfMemory();
    }

    mini_texture->data = SDL_calloc(1, mini_texture->size);
    texture->driverdata = mini_texture;
    update_texture(NULL, texture, NULL, 0);
    GFX_Clear();
    return 0;
}

static int Mini_LockTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect, void **pixels, int *pitch)
{
    Mini_TextureData *mini_texture = (Mini_TextureData*)texture->driverdata;

    *pixels = mini_texture->data;
    *pitch = mini_texture->pitch;
    return 0;
}

static int Mini_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect, const void *pixels, int pitch)
{
    update_texture(texture, texture, pixels, pitch);
    return 0;
}

static void Mini_UnlockTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    SDL_Rect rect = {0};
    Mini_TextureData *mini_texture = (Mini_TextureData*)texture->driverdata;

    rect.x = 0;
    rect.y = 0;
    rect.w = texture->w;
    rect.h = texture->h;
    Mini_UpdateTexture(renderer, texture, &rect, mini_texture->data, mini_texture->pitch);
}

static void Mini_SetTextureScaleMode(SDL_Renderer *renderer, SDL_Texture *texture, SDL_ScaleMode scaleMode)
{
}

static int Mini_SetRenderTarget(SDL_Renderer *renderer, SDL_Texture *texture)
{
    return 0;
}

static int Mini_QueueSetViewport(SDL_Renderer *renderer, SDL_RenderCommand *cmd)
{
    return 0;
}

static int Mini_QueueDrawPoints(SDL_Renderer *renderer, SDL_RenderCommand *cmd, const SDL_FPoint *points, int count)
{
    return 0;
}

static int Mini_QueueGeometry(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture,
                                const float *xy, int xy_stride, const SDL_Color *color, int color_stride, const float *uv, int uv_stride,
                                int num_vertices, const void *indices, int num_indices, int size_indices,
                                float scale_x, float scale_y)
{
    return 0;
}

static int Mini_QueueFillRects(SDL_Renderer *renderer, SDL_RenderCommand *cmd, const SDL_FRect *rects, int count)
{
    return 0;
}

static int Mini_QueueCopy(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_FRect *dstrect)
{
    if (evt.mode == Mini_MOUSE_MODE) {
        threading_mode = 0;
        return My_QueueCopy(texture, get_pixels(texture), srcrect, dstrect);
    }
    
    threading_mode = 1;
    gfx.thread.texture = texture;
    gfx.thread.srt.x = srcrect->x;
    gfx.thread.srt.y = srcrect->y;
    gfx.thread.srt.w = srcrect->w;
    gfx.thread.srt.h = srcrect->h;
    gfx.thread.drt.x = dstrect->x;
    gfx.thread.drt.y = dstrect->y;
    gfx.thread.drt.w = dstrect->w;
    gfx.thread.drt.h = dstrect->h;
    return 0;
}

int My_QueueCopy(SDL_Texture *texture, const void *pixels, const SDL_Rect *srcrect, const SDL_FRect *dstrect)
{
    int pitch = 0;
    SDL_Rect dst = {dstrect->x, dstrect->y, dstrect->w, dstrect->h};
    SDL_Rect src = {srcrect->x, srcrect->y, srcrect->w, srcrect->h};

    pitch = get_pitch(texture);
    if ((pitch == 0) || (pixels == NULL)) {
        return 0;
    }

    evt.mouse.minx = dstrect->x;
    evt.mouse.miny = dstrect->y;
    evt.mouse.maxx = evt.mouse.minx + dstrect->w;
    evt.mouse.maxy = evt.mouse.miny + dstrect->h;
    if (0 && (evt.mode == Mini_MOUSE_MODE)) {
        draw_pen(pixels, src.w, pitch);
    }

    dst.w = 640;
    dst.h = 480;
    GFX_Copy(pixels, src, dst, pitch, 0, E_MI_GFX_ROTATE_180);
    return 0;
}

static int Mini_QueueCopyEx(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture,
    const SDL_Rect *srcrect, const SDL_FRect *dstrect, const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip)
{
    return 0;
}

static int Mini_RunCommandQueue(SDL_Renderer *renderer, SDL_RenderCommand *cmd, void *vertices, size_t vertsize)
{
    return 0;
}

static int Mini_RenderReadPixels(SDL_Renderer *renderer, const SDL_Rect *rect, Uint32 pixel_format, void *pixels, int pitch)
{
    return SDL_Unsupported();
}

static void Mini_RenderPresent(SDL_Renderer *renderer)
{
    if (threading_mode > 0) {
        gfx.action = GFX_ACTION_FLIP;
    }
    else {
        GFX_Flip();
    }
}

static void Mini_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    Mini_TextureData *mini_texture = (Mini_TextureData*)texture->driverdata;

    if (mini_texture) {
        update_texture(texture, NULL, NULL, 0);
        if (mini_texture->data) {
            SDL_free(mini_texture->data);
        }
        SDL_free(mini_texture);
        texture->driverdata = NULL;
    }
}

static void Mini_DestroyRenderer(SDL_Renderer *renderer)
{
    Mini_RenderData *data = (Mini_RenderData *)renderer->driverdata;

    if(data) {
        if(!data->initialized) {
            return;
        }

        data->initialized = SDL_FALSE;
        SDL_free(data);
    }
    SDL_free(renderer);
}

static int Mini_SetVSync(SDL_Renderer *renderer, const int vsync)
{
    return 0;
}

SDL_Renderer *Mini_CreateRenderer(SDL_Window *window, Uint32 flags)
{
    int pixelformat = 0;
    SDL_Renderer *renderer = NULL;
    Mini_RenderData *data = NULL;

    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
    if(!renderer) {
        printf(PREFIX"Failed to create render\n");
        SDL_OutOfMemory();
        return NULL;
    }

    data = (Mini_RenderData *) SDL_calloc(1, sizeof(*data));
    if(!data) {
        printf(PREFIX"Failed to create render data\n");
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
    renderer->info.flags = (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    renderer->driverdata = data;
    renderer->window = window;

    if(data->initialized != SDL_FALSE) {
        return 0;
    }
    data->initialized = SDL_TRUE;

    if(flags & SDL_RENDERER_PRESENTVSYNC) {
        data->vsync = SDL_TRUE;
    }
    else {
        data->vsync = SDL_FALSE;
    }

    pixelformat = SDL_GetWindowPixelFormat(window);
    switch(pixelformat) {
    case SDL_PIXELFORMAT_RGB565:
        data->bpp = 2;
        break;
    case SDL_PIXELFORMAT_ARGB8888:
        data->bpp = 4;
        break;
    }
    return renderer;
}

SDL_RenderDriver Mini_RenderDriver = {
    .CreateRenderer = Mini_CreateRenderer,
    .info = {
        .name = "Mini",
        .flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE,
        .num_texture_formats = 2,
        .texture_formats = {
            [0] = SDL_PIXELFORMAT_RGB565, [2] = SDL_PIXELFORMAT_ARGB8888,
        },
        .max_texture_width = 800,
        .max_texture_height = 600,
    }
};

#endif

