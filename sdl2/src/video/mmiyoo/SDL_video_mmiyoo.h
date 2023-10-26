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

#ifndef __SDL_VIDEO_MMIYOO_H__
#define __SDL_VIDEO_MMIYOO_H__

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
#include "SDL_video_mmiyoo.h"
#include "SDL_event_mmiyoo.h"
#include "SDL_framebuffer_mmiyoo.h"
#include "SDL_opengles_mmiyoo.h"

#include "../../audio/mmiyoo/mi_sys.h"
#include "../../video/mmiyoo/mi_gfx.h"

#ifndef MAX_PATH
    #define MAX_PATH 255
#endif

#define FB_W                        640
#define FB_H                        480
#define FB_BPP                      4
#define FB_SIZE                     (FB_W * FB_H * FB_BPP * 2)
#define TMP_SIZE                    (FB_W * FB_H * FB_BPP)
#define MMIYOO_DRIVER_NAME          "mmiyoo"
#define BASE_REG_RIU_PA             0x1f000000
#define BASE_REG_MPLL_PA            (BASE_REG_RIU_PA + 0x103000 * 2)

typedef struct MMIYOO_VideoInfo {
    SDL_Window *window;
} MMIYOO_VideoInfo;

typedef struct _GFX {
    int fd;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    struct _DMA {
        MI_PHY phyAddr;
        void *virAddr;
    } fb, tmp, overlay;

    struct _HW {
        struct _BUF {
            MI_GFX_Surface_t surf;
            MI_GFX_Rect_t rt;
        } src, dst, overlay;
        MI_GFX_Opt_t opt;
    } hw;
} GFX;

void GFX_Clear(void);
void GFX_Flip(void);
int GFX_Copy(const void *pixels, SDL_Rect srcrect, SDL_Rect dstrect, int pitch, int alpha, int rotate);

int draw_pen(const void *pixels, int width, int pitch);

#endif

