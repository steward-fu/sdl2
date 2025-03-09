// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_MINI

#include <time.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>

#include "../../events/SDL_events_c.h"
#include "../SDL_sysvideo.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"

#include "SDL_image.h"
#include "SDL_version.h"
#include "SDL_syswm.h"
#include "SDL_loadso.h"
#include "SDL_events.h"
#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_video_mini.h"
#include "SDL_event_mini.h"
#include "SDL_gles_mini.h"
#include "SDL_fb_mini.h"

GFX gfx = {0};
Mini_VideoInfo vid={0};

int FB_W = 0;
int FB_H = 0;
int FB_SIZE = 0;
int TMP_SIZE = 0;
int need_screen_rotation_helper = 0;

static pthread_t thread;
static int is_running = 0;
static SDL_Surface *cvt = NULL;

extern Mini_EventInfo evt;

static int Mini_VideoInit(_THIS);
static int Mini_SetDisplayMode(_THIS, SDL_VideoDisplay *display, SDL_DisplayMode *mode);
static void Mini_VideoQuit(_THIS);

static void *video_handler(void *threadid)
{
    debug("%s\n", __func__);
    while (is_running) {
        if (gfx.action == GFX_ACTION_FLIP) {
            gfx.action = GFX_ACTION_NONE;
            My_QueueCopy(gfx.thread.texture, get_pixels(gfx.thread.texture), &gfx.thread.srt, &gfx.thread.drt);
            GFX_Flip();
        }
        usleep(1);
    }
    pthread_exit(NULL);
}

int fb_init(void)
{
    debug("%s\n", __func__);
    MI_SYS_Init();
    MI_GFX_Open();

    gfx.fb_dev = open("/dev/fb0", O_RDWR);
    ioctl(gfx.fb_dev, FBIOGET_FSCREENINFO, &gfx.finfo);
    ioctl(gfx.fb_dev, FBIOGET_VSCREENINFO, &gfx.vinfo);
    gfx.vinfo.yoffset = 0;
    gfx.vinfo.yres_virtual = gfx.vinfo.yres * 2;
    ioctl(gfx.fb_dev, FBIOPUT_VSCREENINFO, &gfx.vinfo);

    gfx.fb.phyAddr = gfx.finfo.smem_start;
    MI_SYS_MemsetPa(gfx.fb.phyAddr, 0, FB_SIZE);
    MI_SYS_Mmap(gfx.fb.phyAddr, gfx.finfo.smem_len, &gfx.fb.virAddr, TRUE);
    memset(&gfx.hw.opt, 0, sizeof(gfx.hw.opt));

    MI_SYS_MMA_Alloc(NULL, TMP_SIZE, &gfx.tmp.phyAddr);
    MI_SYS_Mmap(gfx.tmp.phyAddr, TMP_SIZE, &gfx.tmp.virAddr, TRUE);

    MI_SYS_MMA_Alloc(NULL, TMP_SIZE, &gfx.overlay.phyAddr);
    MI_SYS_Mmap(gfx.overlay.phyAddr, TMP_SIZE, &gfx.overlay.virAddr, TRUE);
    return 0;
}

int fb_quit(void)
{
    debug("%s\n", __func__);
    MI_SYS_Munmap(gfx.fb.virAddr, TMP_SIZE);
    MI_SYS_Munmap(gfx.tmp.virAddr, TMP_SIZE);
    MI_SYS_MMA_Free(gfx.tmp.phyAddr);
    MI_SYS_Munmap(gfx.overlay.virAddr, TMP_SIZE);
    MI_SYS_MMA_Free(gfx.overlay.phyAddr);

    MI_GFX_Close();
    MI_SYS_Exit();
    return 0;
}

void GFX_Init(void)
{
    debug("%s\n", __func__);
    fb_init();
    gfx.thread.pixels = malloc(TMP_SIZE);
    cvt = SDL_CreateRGBSurface(SDL_SWSURFACE, FB_W, FB_H, 32, 0, 0, 0, 0);

    is_running = 1;
    gfx.action = GFX_ACTION_NONE;
    pthread_create(&thread, NULL, video_handler, (void *)NULL);
}

void GFX_Quit(void)
{
    void *ret = NULL;

    debug("%s\n", __func__);
    is_running = 0;
    pthread_join(thread, &ret);
    GFX_Clear();

    fb_quit();
    if (gfx.thread.pixels) {
        free(gfx.thread.pixels);
        gfx.thread.pixels = NULL;
    }

    gfx.vinfo.yoffset = 0;
    ioctl(gfx.fb_dev, FBIOPUT_VSCREENINFO, &gfx.vinfo);
    close(gfx.fb_dev);
    gfx.fb_dev = 0;

    if (cvt) {
        SDL_FreeSurface(cvt);
        cvt = NULL;
    }
}

void GFX_Clear(void)
{
    debug("%s\n", __func__);
    MI_SYS_MemsetPa(gfx.fb.phyAddr, 0, FB_SIZE);
    MI_SYS_MemsetPa(gfx.tmp.phyAddr, 0, TMP_SIZE);
}

int GFX_Copy(const void *pixels, SDL_Rect srcrect, SDL_Rect dstrect, int pitch, int alpha, int rotate)
{
    int copy_it = 1;
    MI_U16 u16Fence = 0;
    int is_rgb565 = (pitch / srcrect.w) == 2 ? 1 : 0;

    debug("%s\n", __func__);
    if (pixels == NULL) {
        return -1;
    }

    if (copy_it) {
        memcpy(gfx.tmp.virAddr, pixels, srcrect.h * pitch);
    }

    gfx.hw.opt.u32GlobalSrcConstColor = 0;
    gfx.hw.opt.eRotate = rotate;
    gfx.hw.opt.eSrcDfbBldOp = E_MI_GFX_DFB_BLD_ONE;
    gfx.hw.opt.eDstDfbBldOp = 0;
    gfx.hw.opt.eDFBBlendFlag = 0;

    gfx.hw.src.rt.s32Xpos = srcrect.x;
    gfx.hw.src.rt.s32Ypos = srcrect.y;
    gfx.hw.src.rt.u32Width = srcrect.w;
    gfx.hw.src.rt.u32Height = srcrect.h;
    gfx.hw.src.surf.u32Width = srcrect.w;
    gfx.hw.src.surf.u32Height = srcrect.h;
    gfx.hw.src.surf.u32Stride = pitch;
    gfx.hw.src.surf.eColorFmt = is_rgb565 ? E_MI_GFX_FMT_RGB565 : E_MI_GFX_FMT_ARGB8888;
    gfx.hw.src.surf.phyAddr = gfx.tmp.phyAddr;

    gfx.hw.dst.rt.s32Xpos = 0;
    gfx.hw.dst.rt.s32Ypos = 0;
    gfx.hw.dst.rt.u32Width = FB_W;
    gfx.hw.dst.rt.u32Height = FB_H;
    gfx.hw.dst.rt.s32Xpos = dstrect.x;
    gfx.hw.dst.rt.s32Ypos = dstrect.y;
    gfx.hw.dst.rt.u32Width = dstrect.w;
    gfx.hw.dst.rt.u32Height = dstrect.h;
    gfx.hw.dst.surf.u32Width = FB_W;
    gfx.hw.dst.surf.u32Height = FB_H;
    gfx.hw.dst.surf.u32Stride = FB_W * FB_BPP;
    gfx.hw.dst.surf.eColorFmt = E_MI_GFX_FMT_ARGB8888;
    gfx.hw.dst.surf.phyAddr = gfx.fb.phyAddr + (FB_W * gfx.vinfo.yoffset * FB_BPP);

    MI_SYS_FlushInvCache(gfx.tmp.virAddr, pitch * srcrect.h);
    MI_GFX_BitBlit(&gfx.hw.src.surf, &gfx.hw.src.rt, &gfx.hw.dst.surf, &gfx.hw.dst.rt, &gfx.hw.opt, &u16Fence);
    MI_GFX_WaitAllDone(FALSE, u16Fence);
    return 0;
}

void GFX_Flip(void)
{
    debug("%s\n", __func__);
    ioctl(gfx.fb_dev, FBIOPAN_DISPLAY, &gfx.vinfo);
    gfx.vinfo.yoffset ^= FB_H;
}

void* GFX_CB(void)
{
    debug("%s\n", __func__);
    return NULL;
}

static void Mini_DeleteDevice(SDL_VideoDevice *device)
{
    debug("%s\n", __func__);
    SDL_free(device);
}

void Mini_DestroyWindow(_THIS, SDL_Window *window)
{
    debug("%s\n", __func__);
    GFX_Quit();
}

int Mini_CreateWindow(_THIS, SDL_Window *window)
{
    vid.window = window;
    SDL_SetMouseFocus(window);
    glUpdateBufferSettings(GFX_CB);
    debug("%s, win=%p, w=%d, h=%d\n", __func__, window, window->w, window->h);
    return 0;
}

int Mini_CreateWindowFrom(_THIS, SDL_Window *window, const void *data)
{
    debug("%s\n", __func__);
    return SDL_Unsupported();
}

static SDL_VideoDevice *Mini_CreateDevice(int devindex)
{
    SDL_VideoDevice *device = NULL;
    SDL_GLDriverData *gldata = NULL;

    debug("%s\n", __func__);

    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (!device) {
        SDL_OutOfMemory();
        return NULL;
    }

    device->is_dummy = SDL_TRUE;
    device->VideoInit = Mini_VideoInit;
    device->VideoQuit = Mini_VideoQuit;
    device->SetDisplayMode = Mini_SetDisplayMode;
    device->PumpEvents = Mini_PumpEvents;
    device->CreateSDLWindow = Mini_CreateWindow;
    device->CreateSDLWindowFrom = Mini_CreateWindowFrom;
    device->CreateWindowFramebuffer = Mini_CreateWindowFramebuffer;
    device->UpdateWindowFramebuffer = Mini_UpdateWindowFramebuffer;
    device->DestroyWindowFramebuffer = Mini_DestroyWindowFramebuffer;
    device->DestroyWindow = Mini_DestroyWindow;

    device->GL_LoadLibrary = glLoadLibrary;
    device->GL_GetProcAddress = glGetProcAddress;
    device->GL_CreateContext = glCreateContext;
    device->GL_SetSwapInterval = glSetSwapInterval;
    device->GL_SwapWindow = glSwapWindow;
    device->GL_MakeCurrent = glMakeCurrent;
    device->GL_DeleteContext = glDeleteContext;
    device->GL_UnloadLibrary = glUnloadLibrary;
    
    gldata = (SDL_GLDriverData*)SDL_calloc(1, sizeof(SDL_GLDriverData));
    if (gldata == NULL) {
        SDL_OutOfMemory();
        SDL_free(device);
        return NULL;
    }

    device->gl_data = gldata;
    device->free = Mini_DeleteDevice;
    return device;
}

VideoBootStrap Mini_bootstrap = { "Mini", "Miyoo Mini Video Driver", Mini_CreateDevice };

int Mini_VideoInit(_THIS)
{
    FILE *fd = NULL;
    char buf[MAX_PATH] = {0};
    SDL_DisplayMode mode = {0};
    SDL_VideoDisplay display = {0};

    debug("%s\n", __func__);

    SDL_zero(mode);
    mode.format = SDL_PIXELFORMAT_RGB565;
    mode.w = 640;
    mode.h = 480;
    mode.refresh_rate = 60;
    SDL_AddDisplayMode(&display, &mode);

    SDL_zero(mode);
    mode.format = SDL_PIXELFORMAT_ARGB8888;
    mode.w = 640;
    mode.h = 480;
    mode.refresh_rate = 60;
    SDL_AddDisplayMode(&display, &mode);

    SDL_zero(mode);
    mode.format = SDL_PIXELFORMAT_RGB565;
    mode.w = 800;
    mode.h = 480;
    mode.refresh_rate = 60;
    SDL_AddDisplayMode(&display, &mode);

    SDL_zero(mode);
    mode.format = SDL_PIXELFORMAT_ARGB8888;
    mode.w = 800;
    mode.h = 480;
    mode.refresh_rate = 60;
    SDL_AddDisplayMode(&display, &mode);
    SDL_zero(mode);
    mode.format = SDL_PIXELFORMAT_RGB565;
    mode.w = 800;
    mode.h = 600;
    mode.refresh_rate = 60;
    SDL_AddDisplayMode(&display, &mode);

    SDL_zero(mode);
    mode.format = SDL_PIXELFORMAT_ARGB8888;
    mode.w = 800;
    mode.h = 600;
    mode.refresh_rate = 60;
    SDL_AddDisplayMode(&display, &mode);

    SDL_zero(mode);
    mode.format = SDL_PIXELFORMAT_RGB565;
    mode.w = 320;
    mode.h = 240;
    mode.refresh_rate = 60;
    SDL_AddDisplayMode(&display, &mode);

    SDL_zero(mode);
    mode.format = SDL_PIXELFORMAT_ARGB8888;
    mode.w = 320;
    mode.h = 240;
    mode.refresh_rate = 60;
    SDL_AddDisplayMode(&display, &mode);

    SDL_zero(mode);
    mode.format = SDL_PIXELFORMAT_RGB565;
    mode.w = 480;
    mode.h = 272;
    mode.refresh_rate = 60;
    SDL_AddDisplayMode(&display, &mode);

    SDL_zero(mode);
    mode.format = SDL_PIXELFORMAT_ARGB8888;
    mode.w = 480;
    mode.h = 272;
    mode.refresh_rate = 60;
    SDL_AddDisplayMode(&display, &mode);
    SDL_AddVideoDisplay(&display, SDL_FALSE);

    FB_W = DEF_FB_W;
    FB_H = DEF_FB_H;
    FB_SIZE = (FB_W * FB_H * FB_BPP * 2);
    TMP_SIZE = (FB_W * FB_H * FB_BPP);
    fd = popen("fbset | grep \"mode \"", "r");
    if (fd) {
        fgets(buf, sizeof(buf), fd);
        pclose(fd);

        if (strstr(buf, "752")) {
            FB_W = 752;
            FB_H = 560;
            FB_SIZE = (FB_W * FB_H * FB_BPP * 2);
            TMP_SIZE = (FB_W * FB_H * FB_BPP);
        }
    }

    GFX_Init();
    Mini_EventInit();
    return 0;
}

static int Mini_SetDisplayMode(_THIS, SDL_VideoDisplay *display, SDL_DisplayMode *mode)
{
    debug("%s\n", __func__);
    return 0;
}

void Mini_VideoQuit(_THIS)
{
    debug("%s\n", __func__);
    Mini_EventDeinit();
}

#endif

