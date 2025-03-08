// LGPL-2.1 License
// // (C) 2025 Steward Fu <steward.fu@gmail.com>

#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_XT897

#include "../SDL_sysvideo.h"

#include "SDL_video_xt897.h"
#include "SDL_fb_xt897.h"

int XT897_CreateWindowFramebuffer(_THIS, SDL_Window *window, Uint32 *format, void **pixels, int *pitch)
{
    debug("%s\n", __func__);
    return 0;
}

int XT897_UpdateWindowFramebuffer(_THIS, SDL_Window *window, const SDL_Rect *rects, int numrects)
{
    debug("%s\n", __func__);
    return 0;
}

void XT897_DestroyWindowFramebuffer(_THIS, SDL_Window *window)
{
    debug("%s\n", __func__);
}

#endif

