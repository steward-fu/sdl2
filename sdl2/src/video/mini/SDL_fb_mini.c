// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_MINI

#include "../SDL_sysvideo.h"

#include "SDL_video_mini.h"
#include "SDL_fb_mini.h"

#define MINI_SURFACE "SDL_MiniSurface"

int Mini_CreateWindowFramebuffer(_THIS, SDL_Window *window, Uint32 *format, void **pixels, int *pitch)
{
    int w = 0;
    int h = 0;
    int bpp = 0;
    uint32_t Rmask = 0;
    uint32_t Gmask = 0;
    uint32_t Bmask = 0;
    uint32_t Amask = 0;
    SDL_Surface *surface = NULL;
    const uint32_t surface_format = SDL_PIXELFORMAT_RGB888;

    debug("%s\n", __func__);
    surface = (SDL_Surface *) SDL_GetWindowData(window, MINI_SURFACE);
    SDL_FreeSurface(surface);

    SDL_PixelFormatEnumToMasks(surface_format, &bpp, &Rmask, &Gmask, &Bmask, &Amask);
    SDL_GetWindowSize(window, &w, &h);
    surface = SDL_CreateRGBSurface(0, w, h, bpp, Rmask, Gmask, Bmask, Amask);
    if (!surface) {
        debug("%s, failed to create window surface\n", __func__);
        return -1;
    }

    SDL_SetWindowData(window, MINI_SURFACE, surface);
    *format = surface_format;
    *pixels = surface->pixels;
    *pitch = surface->pitch;
    return 0;
}

int Mini_UpdateWindowFramebuffer(_THIS, SDL_Window *window, const SDL_Rect *rects, int numrects)
{
    debug("%s\n", __func__);
    return 0;
}

void Mini_DestroyWindowFramebuffer(_THIS, SDL_Window *window)
{
    SDL_Surface *surface = (SDL_Surface *) SDL_SetWindowData(window, MINI_SURFACE, NULL);

    debug("%s\n", __func__);
    SDL_FreeSurface(surface);
}

#endif

