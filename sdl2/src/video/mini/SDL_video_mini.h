// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#ifndef __SDL_VIDEO_Mini_H__
#define __SDL_VIDEO_Mini_H__

#include "../../SDL_internal.h"

#include <stdint.h>
#include <stdbool.h>
#include <linux/fb.h>

#include "../SDL_sysvideo.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_ttf.h"
#include "SDL_image.h"
#include "SDL_version.h"
#include "SDL_syswm.h"
#include "SDL_loadso.h"
#include "SDL_events.h"
#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_video_mini.h"
#include "SDL_event_mini.h"
#include "SDL_fb_mini.h"
#include "SDL_gles_mini.h"

#if 1
    #define debug(...) printf(__VA_ARGS__)
#else
    #define debug(...) (void)0
#endif

#include "mi_sys.h"
#include "mi_gfx.h"

#ifndef MAX_PATH
    #define MAX_PATH 128
#endif

#define DEF_FB_W                640
#define DEF_FB_H                480

#define PREFIX                      "[SDL] "
#define FB_BPP                      4
#define IMG_W                       640
#define IMG_H                       480
#define GFX_ACTION_NONE             0
#define GFX_ACTION_FLIP             1
#define GFX_ACTION_COPY0            2
#define GFX_ACTION_COPY1            3

typedef struct Mini_VideoInfo {
    SDL_Window *window;
} Mini_VideoInfo;

typedef struct _GFX {
    int fb_dev;

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    struct _DMA {
        void *virAddr;
        MI_PHY phyAddr;
    } fb, tmp, overlay;

    struct _HW {
        struct _BUF {
            MI_GFX_Surface_t surf;
            MI_GFX_Rect_t rt;
        } src, dst, overlay;
        MI_GFX_Opt_t opt;
    } hw;

    int action;
    struct _THREAD {
        void *pixels;
        SDL_Rect srt;
        SDL_FRect drt;
        SDL_Texture *texture;
    } thread;
} GFX;

void GFX_Clear(void);
void GFX_Flip(void);
int GFX_Copy(const void *pixels, SDL_Rect srcrect, SDL_Rect dstrect, int pitch, int alpha, int rotate);

int fb_init(void);
int fb_uninit(void);

int draw_pen(const void *pixels, int width, int pitch);
int My_QueueCopy(SDL_Texture *texture, const void *pixels, const SDL_Rect *srcrect, const SDL_FRect *dstrect);
const void* get_pixels(void *chk);

#endif

