// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#ifndef __SDL_XT897_FB_H__
#define __SDL_XT897_FB_H__

#if SDL_VIDEO_DRIVER_XT897

int XT897_CreateWindowFramebuffer(_THIS, SDL_Window *window, Uint32 *format, void **pixels, int *pitch);
int XT897_UpdateWindowFramebuffer(_THIS, SDL_Window *window, const SDL_Rect *rects, int numrects);
void XT897_DestroyWindowFramebuffer(_THIS, SDL_Window *window);

#endif

#endif

