// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_MINI

#include "../SDL_sysvideo.h"
#include "SDL_fb_mini.h"

#define MINI_SURFACE "_SDL_MiniSurface"

int Mini_CreateWindowFramebuffer(_THIS, SDL_Window *window, Uint32 *format, void **pixels, int *pitch)
{
    SDL_Surface *surface;
    const Uint32 surface_format = SDL_PIXELFORMAT_RGB888;
    int w, h;
    int bpp;
    Uint32 Rmask, Gmask, Bmask, Amask;

    surface = (SDL_Surface *) SDL_GetWindowData(window, MINI_SURFACE);
    SDL_FreeSurface(surface);

    SDL_PixelFormatEnumToMasks(surface_format, &bpp, &Rmask, &Gmask, &Bmask, &Amask);
    SDL_GetWindowSize(window, &w, &h);
    surface = SDL_CreateRGBSurface(0, w, h, bpp, Rmask, Gmask, Bmask, Amask);
    if(!surface) {
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
    static int frame_number;
    SDL_Surface *surface;

    surface = (SDL_Surface *) SDL_GetWindowData(window, MINI_SURFACE);
    if(!surface) {
        return SDL_SetError("Couldn't find mini surface for window");
    }

    if(SDL_getenv("SDL_VIDEO_Mini_SAVE_FRAMES")) {
        char file[128];
        SDL_snprintf(file, sizeof(file), "SDL_window%" SDL_PRIu32 "-%8.8d.bmp",
                     SDL_GetWindowID(window), ++frame_number);
        SDL_SaveBMP(surface, file);
    }
    return 0;
}

void Mini_DestroyWindowFramebuffer(_THIS, SDL_Window *window)
{
    SDL_Surface *surface;

    surface = (SDL_Surface *) SDL_SetWindowData(window, MINI_SURFACE, NULL);
    SDL_FreeSurface(surface);
}

#endif

