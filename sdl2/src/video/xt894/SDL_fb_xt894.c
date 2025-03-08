// LGPL-2.1 License
// // (C) 2025 Steward Fu <steward.fu@gmail.com>

#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_XT894

#include "../SDL_sysvideo.h"

#include "SDL_video_xt894.h"
#include "SDL_fb_xt894.h"

int XT894_CreateWindowFramebuffer(_THIS, SDL_Window *window, Uint32 *format, void **pixels, int *pitch)
{
    debug("%s\n", __func__);
    return 0;
}

int XT894_UpdateWindowFramebuffer(_THIS, SDL_Window *window, const SDL_Rect *rects, int numrects)
{
    debug("%s\n", __func__);
    return 0;
}

void XT894_DestroyWindowFramebuffer(_THIS, SDL_Window *window)
{
    debug("%s\n", __func__);
}

#endif

