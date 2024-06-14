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

#if SDL_VIDEO_RENDER_A30

#include <unistd.h>
#include <stdbool.h>

#include "SDL_hints.h"
#include "../SDL_sysrender.h"
#include "../../video/a30/SDL_video_a30.h"
#include "../../video/a30/SDL_event_a30.h"

typedef struct A30_TextureData {
    void *data;
    unsigned int size;
    unsigned int width;
    unsigned int height;
    unsigned int bits;
    unsigned int format;
    unsigned int pitch;
} A30_TextureData;

typedef struct {
    SDL_Texture *boundTarget;
    SDL_bool initialized;
    unsigned int bpp;
    SDL_bool vsync;
} A30_RenderData;

SDL_RenderDriver A30_RenderDriver;

extern struct _video vid;
extern GLfloat bgVertices[];
extern GLfloat vVertices[];
extern GLushort indices[];

static void A30_WindowEvent(SDL_Renderer *renderer, const SDL_WindowEvent *event)
{
    printf(PREFIX"%s\n", __func__);
}

static int A30_CreateTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    A30_TextureData *a30_texture = (A30_TextureData *)SDL_calloc(1, sizeof(*a30_texture));

    printf(PREFIX"%s\n", __func__);
    if(!a30_texture) {
        printf(PREFIX"Failed to create texture\n");
        return SDL_OutOfMemory();
    }

    a30_texture->width = texture->w;
    a30_texture->height = texture->h;
    a30_texture->format = texture->format;

    switch(a30_texture->format) {
    case SDL_PIXELFORMAT_RGB565:
        a30_texture->bits = 16;
        break;
    case SDL_PIXELFORMAT_ARGB8888:
        a30_texture->bits = 32;
        break;
    default:
        return -1;
    }

    a30_texture->pitch = a30_texture->width * SDL_BYTESPERPIXEL(texture->format);
    a30_texture->size = a30_texture->height * a30_texture->pitch;
    a30_texture->data = SDL_calloc(1, a30_texture->size);

    if(!a30_texture->data) {
        printf(PREFIX"Failed to create texture data\n");
        SDL_free(a30_texture);
        return SDL_OutOfMemory();
    }

    a30_texture->data = SDL_calloc(1, a30_texture->size);
    texture->driverdata = a30_texture;
    return 0;
}

static int A30_LockTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect, void **pixels, int *pitch)
{
    A30_TextureData *a30_texture = (A30_TextureData*)texture->driverdata;

    *pixels = a30_texture->data;
    *pitch = a30_texture->pitch;
    return 0;
}

static int A30_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect, const void *pixels, int pitch)
{
    int x = 0;
    int y = 0;
    int w = rect->w;
    int h = rect->h;
    float w0 = (float)LCD_W / w;
    float h0 = (float)LCD_H / h;

    w = (float)w * ((w0 > h0) ? h0 : w0);
    h = (float)h * ((w0 > h0) ? h0 : w0);
    x = (LCD_W - w) / 2;
    y = (LCD_H - h) / 2;

    glBindTexture(GL_TEXTURE_2D, vid.texID[TEX_SCR]);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if ((pitch / rect->w) == 2) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rect->w, rect->h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pixels);
    }
    else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rect->w, rect->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    }

    if (vid.scale) {
        vVertices[0] = ((((float)x) / LCD_W) - 0.5) * 2.0;
        vVertices[1] = ((((float)y) / LCD_H) - 0.5) * -2.0;

        vVertices[5] = vVertices[0];
        vVertices[6] = ((((float)(y + h)) / LCD_H) - 0.5) * -2.0;

        vVertices[10] = ((((float)(x + w)) / LCD_W) - 0.5) * 2.0;
        vVertices[11] = vVertices[6];

        vVertices[15] = vVertices[10];
        vVertices[16] = vVertices[1];
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, vid.texID[TEX_SCR]);
    glVertexAttribPointer(vid.posLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vVertices);
    glVertexAttribPointer(vid.texLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    glUniform1f(vid.alphaLoc, 0.0);
    glDisable(GL_BLEND);
    return 0;
}

static void A30_UnlockTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    SDL_Rect rect = {0};
    A30_TextureData *a30_texture = (A30_TextureData*)texture->driverdata;

    rect.x = 0;
    rect.y = 0;
    rect.w = texture->w;
    rect.h = texture->h;
    A30_UpdateTexture(renderer, texture, &rect, a30_texture->data, a30_texture->pitch);
}

static void A30_SetTextureScaleMode(SDL_Renderer *renderer, SDL_Texture *texture, SDL_ScaleMode scaleMode)
{
    printf(PREFIX"%s\n", __func__);
}

static int A30_SetRenderTarget(SDL_Renderer *renderer, SDL_Texture *texture)
{
    return 0;
}

static int A30_QueueSetViewport(SDL_Renderer *renderer, SDL_RenderCommand *cmd)
{
    return 0;
}

static int A30_QueueDrawPoints(SDL_Renderer *renderer, SDL_RenderCommand *cmd, const SDL_FPoint *points, int count)
{
    return 0;
}

static int A30_QueueGeometry(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture,
                                const float *xy, int xy_stride, const SDL_Color *color, int color_stride, const float *uv, int uv_stride,
                                int num_vertices, const void *indices, int num_indices, int size_indices,
                                float scale_x, float scale_y)
{
    return 0;
}

static int A30_QueueFillRects(SDL_Renderer *renderer, SDL_RenderCommand *cmd, const SDL_FRect *rects, int count)
{
    return 0;
}

static int A30_QueueCopy(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_FRect *dstrect)
{
    return 0;
}

int My_QueueCopy(SDL_Texture *texture, const void *pixels, const SDL_Rect *srcrect, const SDL_FRect *dstrect)
{
    return 0;
}

static int A30_QueueCopyEx(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture,
    const SDL_Rect *srcrect, const SDL_FRect *dstrect, const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip)
{
    return 0;
}

static int A30_RunCommandQueue(SDL_Renderer *renderer, SDL_RenderCommand *cmd, void *vertices, size_t vertsize)
{
    return 0;
}

static int A30_RenderReadPixels(SDL_Renderer *renderer, const SDL_Rect *rect, Uint32 pixel_format, void *pixels, int pitch)
{
    return SDL_Unsupported();
}

static void A30_RenderPresent(SDL_Renderer *renderer)
{
    eglSwapBuffers(vid.eglDisplay, vid.eglSurface);
}

static void A30_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    A30_TextureData *a30_texture = (A30_TextureData*)texture->driverdata;

    printf(PREFIX"%s\n", __func__);
    if (a30_texture) {
        if (a30_texture->data) {
            SDL_free(a30_texture->data);
        }
        SDL_free(a30_texture);
        texture->driverdata = NULL;
    }
}

static void A30_DestroyRenderer(SDL_Renderer *renderer)
{
    A30_RenderData *data = (A30_RenderData *)renderer->driverdata;

    printf(PREFIX"%s\n", __func__);
    if(data) {
        if(!data->initialized) {
            return;
        }

        data->initialized = SDL_FALSE;
        SDL_free(data);
    }
    SDL_free(renderer);
}

static int A30_SetVSync(SDL_Renderer *renderer, const int vsync)
{
    return 0;
}

SDL_Renderer *A30_CreateRenderer(SDL_Window *window, Uint32 flags)
{
    int pixelformat = 0;
    SDL_Renderer *renderer = NULL;
    A30_RenderData *data = NULL;

    printf(PREFIX"%s\n", __func__);
    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
    if(!renderer) {
        printf(PREFIX"Failed to create render\n");
        SDL_OutOfMemory();
        return NULL;
    }

    data = (A30_RenderData *) SDL_calloc(1, sizeof(*data));
    if(!data) {
        printf(PREFIX"Failed to create render data\n");
        A30_DestroyRenderer(renderer);
        SDL_OutOfMemory();
        return NULL;
    }

    renderer->WindowEvent = A30_WindowEvent;
    renderer->CreateTexture = A30_CreateTexture;
    renderer->UpdateTexture = A30_UpdateTexture;
    renderer->LockTexture = A30_LockTexture;
    renderer->UnlockTexture = A30_UnlockTexture;
    renderer->SetTextureScaleMode = A30_SetTextureScaleMode;
    renderer->SetRenderTarget = A30_SetRenderTarget;
    renderer->QueueSetViewport = A30_QueueSetViewport;
    renderer->QueueSetDrawColor = A30_QueueSetViewport;
    renderer->QueueDrawPoints = A30_QueueDrawPoints;
    renderer->QueueDrawLines = A30_QueueDrawPoints;
    renderer->QueueGeometry = A30_QueueGeometry;
    renderer->QueueFillRects = A30_QueueFillRects;
    renderer->QueueCopy = A30_QueueCopy;
    renderer->QueueCopyEx = A30_QueueCopyEx;
    renderer->RunCommandQueue = A30_RunCommandQueue;
    renderer->RenderReadPixels = A30_RenderReadPixels;
    renderer->RenderPresent = A30_RenderPresent;
    renderer->DestroyTexture = A30_DestroyTexture;
    renderer->DestroyRenderer = A30_DestroyRenderer;
    renderer->SetVSync = A30_SetVSync;
    renderer->info = A30_RenderDriver.info;
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

SDL_RenderDriver A30_RenderDriver = {
    .CreateRenderer = A30_CreateRenderer,
    .info = {
        .name = "A30",
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
