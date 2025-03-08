// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#ifndef __SDL_XT894_FB_H__
#define __SDL_XT894_FB_H__

#if SDL_VIDEO_DRIVER_XT894

int XT894_CreateWindowFramebuffer(_THIS, SDL_Window *window, Uint32 *format, void **pixels, int *pitch);
int XT894_UpdateWindowFramebuffer(_THIS, SDL_Window *window, const SDL_Rect *rects, int numrects);
void XT894_DestroyWindowFramebuffer(_THIS, SDL_Window *window);

#endif

#endif

