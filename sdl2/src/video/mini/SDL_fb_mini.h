// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#ifndef __SDL_FB_Mini_H__
#define __SDL_FB_Mini_H__

#include "../../SDL_internal.h"

int Mini_CreateWindowFramebuffer(_THIS, SDL_Window *window, Uint32 *format, void **pixels, int *pitch);
int Mini_UpdateWindowFramebuffer(_THIS, SDL_Window *window, const SDL_Rect *rects, int numrects);
void Mini_DestroyWindowFramebuffer(_THIS, SDL_Window *window);

#endif

