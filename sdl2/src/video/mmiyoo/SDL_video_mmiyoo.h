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
#include "SDL_video_mmiyoo.h"
#include "SDL_event_mmiyoo.h"
#include "SDL_framebuffer_mmiyoo.h"
#include "SDL_opengles_mmiyoo.h"

#ifdef MMIYOO
#include "mi_sys.h"
#include "mi_gfx.h"
#endif

#ifdef TRIMUI
#include "trimui.h"
#endif

#ifdef QX1000
#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif

#ifndef MMIYOO
    #define E_MI_GFX_ROTATE_90  0
    #define E_MI_GFX_ROTATE_180 0
    #define E_MI_GFX_ROTATE_270 0
#endif

#ifndef MAX_PATH
    #define MAX_PATH 128
#endif

#ifdef MMIYOO
    #define DEF_FB_W                640
    #define DEF_FB_H                480
#endif

#ifdef TRIMUI
    #define DEF_FB_W                320
    #define DEF_FB_H                240
    #define ION_W                   512
    #define ION_H                   384
#endif

#ifdef QX1000
    #define LCD_W                   1080
    #define LCD_H                   2160
    #define DEF_FB_W                640
    #define DEF_FB_H                480
    #define FB_BPP                  4
#endif

#define PREFIX                      "[SDL] "
#define MMIYOO_DRIVER_NAME          "mmiyoo"
#define FB_BPP                      4
#define IMG_W                       640
#define IMG_H                       480
#define GFX_ACTION_NONE             0
#define GFX_ACTION_FLIP             1
#define GFX_ACTION_COPY0            2
#define GFX_ACTION_COPY1            3

#ifdef QX1000
    struct _wayland {
        struct wl_shell *shell;
        struct wl_region *region;
        struct wl_display *display;
        struct wl_surface *surface;
        struct wl_registry *registry;
        struct wl_compositor *compositor;
        struct wl_shell_surface *shell_surface;

        struct _egl {
            EGLConfig config;
            EGLContext context;
            EGLDisplay display;
            EGLSurface surface;

            GLuint vShader;
            GLuint fShader;
            GLuint pObject;

            GLuint textureId;
            GLint positionLoc;
            GLint texCoordLoc;
            GLint samplerLoc;
            struct wl_egl_window *window;
        } egl;
        
        struct _org {
            int w;
            int h;
            int bpp;
            int size;
        } info;

        int init;
        int ready;
        int flip;
        uint8_t *bg;
        uint8_t *data;
        uint16_t *pixels[2];
    };
#endif

typedef struct MMIYOO_VideoInfo {
    SDL_Window *window;
} MMIYOO_VideoInfo;

typedef struct _GFX {
    int fb_dev;

#ifdef TRIMUI
    int ion_dev;
    int mem_dev;
    int disp_dev;
#endif

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    struct _DMA {
#ifdef MMIYOO
        void *virAddr;
        MI_PHY phyAddr;
#endif

#ifdef TRIMUI
        int flip;
#endif
    } fb, tmp, overlay;

    struct _HW {
#ifdef MMIYOO
        struct _BUF {
            MI_GFX_Surface_t surf;
            MI_GFX_Rect_t rt;
        } src, dst, overlay;
        MI_GFX_Opt_t opt;
#endif

#ifdef TRIMUI
        uint32_t *mem;
        disp_layer_config disp;
        disp_layer_config buf;
        ion_alloc_info_t ion;
#endif
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

