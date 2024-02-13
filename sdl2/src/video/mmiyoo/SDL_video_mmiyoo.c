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

#if SDL_VIDEO_DRIVER_MMIYOO

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
#include "SDL_video_mmiyoo.h"
#include "SDL_event_mmiyoo.h"
#include "SDL_opengles_mmiyoo.h"
#include "SDL_framebuffer_mmiyoo.h"

#include "hex_pen.h"

GFX gfx = {0};
MMIYOO_VideoInfo vid={0};

int FB_W = 0;
int FB_H = 0;
int FB_SIZE = 0;
int TMP_SIZE = 0;

static pthread_t thread;
static int is_running = 0;
static SDL_Surface *cvt = NULL;

extern MMIYOO_EventInfo evt;

static int MMIYOO_VideoInit(_THIS);
static int MMIYOO_SetDisplayMode(_THIS, SDL_VideoDisplay *display, SDL_DisplayMode *mode);
static void MMIYOO_VideoQuit(_THIS);

#ifdef TRIMUI
static uint32_t LUT_256x192_S10[256 * 192] = {0};
static uint32_t LUT_256x192_S11[256 * 192] = {0};
static uint32_t LUT_256x192_S00_pixel[256 * 192] = {0};
static uint32_t LUT_256x192_S01_pixel[256 * 192] = {0};
static uint32_t LUT_256x192_S00_blur[256 * 192] = {0};
static uint32_t LUT_256x192_S01_blur[256 * 192] = {0};
#endif

#ifdef QX1000
static struct _wayland wl = {0};

int thread_run = 0;
static pthread_t thread_id[2] = {0};

EGLint egl_attribs[] = {
    EGL_SURFACE_TYPE,
    EGL_WINDOW_BIT,
    EGL_RENDERABLE_TYPE,
    EGL_OPENGL_ES2_BIT,
    EGL_RED_SIZE,
    8,
    EGL_GREEN_SIZE,
    8,
    EGL_BLUE_SIZE,
    8,
    EGL_NONE
};

EGLint ctx_attribs[] = {
    EGL_CONTEXT_CLIENT_VERSION, 
    2, 
    EGL_NONE
};

GLfloat egl_bg_vertices[] = {
    -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 
    -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 
     1.0f,  1.0f, 0.0f, 1.0f, 0.0f
};

GLfloat egl_fb_vertices[] = {
    -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 
    -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 
     0.5f,  0.5f, 0.0f, 1.0f, 0.0f
};

GLushort egl_indices[] = {0, 1, 2, 0, 2, 3};

const char *vShaderSrc =
    "attribute vec4 a_position;   \n"
    "attribute vec2 a_texCoord;   \n"
    "varying vec2 v_texCoord;     \n"
    "void main()                  \n"
    "{                            \n"
    "   gl_Position = a_position; \n"
    "   v_texCoord = a_texCoord;  \n"
    "}                            \n";

const char *fShaderSrc =
    "#ifdef GL_ES                                              \n"
    "precision mediump float;                                  \n"
    "#endif                                                    \n"
    "varying vec2 v_texCoord;                                  \n"
    "uniform float angle;                                      \n"
    "uniform float aspect;                                     \n"
    "uniform sampler2D s_texture;                              \n"
    "const vec2 HALF = vec2(0.5);                              \n"
    "void main()                                               \n"
    "{                                                         \n"
    "    float aSin = sin(angle);                              \n"
    "    float aCos = cos(angle);                              \n"
    "    vec2 tc = v_texCoord;                                 \n"
    "    mat2 rotMat = mat2(aCos, -aSin, aSin, aCos);          \n"
    "    mat2 scaleMat = mat2(aspect, 0.0, 0.0, 1.0);          \n"
    "    mat2 scaleMatInv = mat2(1.0 / aspect, 0.0, 0.0, 1.0); \n"
    "    tc -= HALF.xy;                                        \n"
    "    tc = scaleMatInv * rotMat * scaleMat * tc;            \n"
    "    tc += HALF.xy;                                        \n"
    "    vec3 tex = texture2D(s_texture, tc).bgr;              \n"
    "    gl_FragColor = vec4(tex, 1.0);                        \n"
    "}                                                         \n";

static void cb_remove(void *dat, struct wl_registry *reg, uint32_t id);
static void cb_handle(void *dat, struct wl_registry *reg, uint32_t id, const char *intf, uint32_t ver);

static struct wl_registry_listener cb_global = {
    .global = cb_handle,
    .global_remove = cb_remove
};

void update_wayland_res(int w, int h)
{
    int c0 = 0;
    int c1 = 0;
    int scale = 0;
    float x0 = 0;
    float y0 = 0;

    FB_W = w;
    FB_H = h;
    wl.info.w = FB_W;
    wl.info.h = FB_H;
    wl.info.bpp = 32;
    wl.info.size = wl.info.w * (wl.info.bpp / 8) * wl.info.h;

    c0 = LCD_H / wl.info.w;
    c1 = LCD_W / wl.info.h;
    scale = c0 > c1 ? c1 : c0;

    y0 = ((float)(wl.info.w * scale) / LCD_H);
    x0 = ((float)(wl.info.h * scale) / LCD_W);

    egl_fb_vertices[0] = -x0;
    egl_fb_vertices[1] = y0;
    egl_fb_vertices[5] = -x0;
    egl_fb_vertices[6] = -y0;
    egl_fb_vertices[10] =  x0;
    egl_fb_vertices[11] = -y0;
    egl_fb_vertices[15] =  x0;
    egl_fb_vertices[16] =  y0;

    memset(wl.data, 0, LCD_W * LCD_H * 2);
    wl.pixels[0] = (uint16_t*)wl.data;
    wl.pixels[1] = (uint16_t*)(wl.data + wl.info.size);
    printf(PREFIX"Updated Wayland (width=%d, height=%d, scale=%d)\n", w, h, scale);
}

static void* wl_thread(void* pParam)
{
    while(thread_run){
        if(wl.init && wl.ready){
            wl_display_dispatch(wl.display);
        }
        usleep(100);
    }
    return NULL;
}

static void cb_ping(void *dat, struct wl_shell_surface *shell_surf, uint32_t serial)
{
    wl_shell_surface_pong(shell_surf, serial);
}

static void cb_configure(void *dat, struct wl_shell_surface *shell_surf, uint32_t edges, int32_t w, int32_t h)
{
}

static void cb_popup_done(void *dat, struct wl_shell_surface *shell_surf)
{
}

static const struct wl_shell_surface_listener cb_shell_surf = {
    cb_ping,
    cb_configure,
    cb_popup_done
};

static void cb_handle(void *dat, struct wl_registry *reg, uint32_t id, const char *intf, uint32_t ver)
{
    if (strcmp(intf, "wl_compositor") == 0) {
        wl.compositor = wl_registry_bind(reg, id, &wl_compositor_interface, 1);
    }
    else if (strcmp(intf, "wl_shell") == 0) {
        wl.shell = wl_registry_bind(reg, id, &wl_shell_interface, 1);
    }
}

static void cb_remove(void *dat, struct wl_registry *reg, uint32_t id)
{
}

void egl_free(void)
{
    wl.init = 0;
    wl.ready = 0;
    eglDestroySurface(wl.egl.display, wl.egl.surface);
    eglDestroyContext(wl.egl.display, wl.egl.context);
    wl_egl_window_destroy(wl.egl.window);
    eglTerminate(wl.egl.display);
    glDeleteShader(wl.egl.vShader);
    glDeleteShader(wl.egl.fShader);
    glDeleteProgram(wl.egl.pObject);
}

void wl_free(void)
{
    wl.init = 0;
    wl.ready = 0;
    wl_shell_surface_destroy(wl.shell_surface);
    wl_shell_destroy(wl.shell);
    wl_surface_destroy(wl.surface);
    wl_compositor_destroy(wl.compositor);
    wl_registry_destroy(wl.registry);
    wl_display_disconnect(wl.display);
    free(wl.data);
}

void wl_create(void)
{
    wl.display = wl_display_connect(NULL);
    wl.registry = wl_display_get_registry(wl.display);

    wl_registry_add_listener(wl.registry, &cb_global, NULL);
    wl_display_dispatch(wl.display);
    wl_display_roundtrip(wl.display);

    wl.surface = wl_compositor_create_surface(wl.compositor);
    wl.shell_surface = wl_shell_get_shell_surface(wl.shell, wl.surface);
    wl_shell_surface_set_toplevel(wl.shell_surface);
    wl_shell_surface_add_listener(wl.shell_surface, &cb_shell_surf, NULL);
    
    wl.region = wl_compositor_create_region(wl.compositor);
    wl_region_add(wl.region, 0, 0, LCD_W, LCD_H);
    wl_surface_set_opaque_region(wl.surface, wl.region);
    wl.egl.window = wl_egl_window_create(wl.surface, LCD_W, LCD_H);
}

void egl_create(void)
{
    GLint compiled = 0;
    EGLConfig cfg = 0;
    EGLint major = 0, minor = 0, cnt = 0;

    wl.egl.display = eglGetDisplay((EGLNativeDisplayType)wl.display);
    eglInitialize(wl.egl.display, &major, &minor);
    eglGetConfigs(wl.egl.display, NULL, 0, &cnt);
    eglChooseConfig(wl.egl.display, egl_attribs, &cfg, 1, &cnt);
    wl.egl.surface = eglCreateWindowSurface(wl.egl.display, cfg, wl.egl.window, NULL);
    wl.egl.context = eglCreateContext(wl.egl.display, cfg, EGL_NO_CONTEXT, ctx_attribs);
    eglMakeCurrent(wl.egl.display, wl.egl.surface, wl.egl.surface, wl.egl.context);

    wl.egl.vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(wl.egl.vShader, 1, &vShaderSrc, NULL);
    glCompileShader(wl.egl.vShader);

    glGetShaderiv(wl.egl.vShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint len = 0;
        glGetShaderiv(wl.egl.vShader, GL_INFO_LOG_LENGTH, &len);
        if (len > 1) {
            char* info = malloc(sizeof(char) * len);
            glGetShaderInfoLog(wl.egl.vShader, len, NULL, info);
            printf(PREFIX"Failed to compile vShader: %s\n", info);
            free(info);
        }
    }

    wl.egl.fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(wl.egl.fShader, 1, &fShaderSrc, NULL);
    glCompileShader(wl.egl.fShader);
    
    glGetShaderiv(wl.egl.fShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint len = 0;
        glGetShaderiv(wl.egl.fShader, GL_INFO_LOG_LENGTH, &len);
        if (len > 1) {
            char* info = malloc(sizeof(char) * len);
            glGetShaderInfoLog(wl.egl.fShader, len, NULL, info);
            printf(PREFIX"Failed to compile fShader: %s\n", info);
            free(info);
        }
    }

    wl.egl.pObject = glCreateProgram();
    glAttachShader(wl.egl.pObject, wl.egl.vShader);
    glAttachShader(wl.egl.pObject, wl.egl.fShader);
    glLinkProgram(wl.egl.pObject);
    glUseProgram(wl.egl.pObject);

    wl.egl.positionLoc = glGetAttribLocation(wl.egl.pObject, "a_position");
    wl.egl.texCoordLoc = glGetAttribLocation(wl.egl.pObject, "a_texCoord");
    wl.egl.samplerLoc = glGetUniformLocation(wl.egl.pObject, "s_texture");
    glUniform1f(glGetUniformLocation(wl.egl.pObject, "angle"), 90 * (3.1415 * 2.0) / 360.0);
    glUniform1f(glGetUniformLocation(wl.egl.pObject, "aspect"), 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &wl.egl.textureId);
    glBindTexture(GL_TEXTURE_2D, wl.egl.textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glViewport(0, 0, LCD_W, LCD_H);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glVertexAttribPointer(wl.egl.positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), egl_bg_vertices);
    glVertexAttribPointer(wl.egl.texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &egl_bg_vertices[3]);
    glEnableVertexAttribArray(wl.egl.positionLoc);
    glEnableVertexAttribArray(wl.egl.texCoordLoc);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(wl.egl.samplerLoc, 0);
}

static void* draw_thread(void *pParam)
{
    wl_create();
    egl_create();

    wl.init = 1;
    while(thread_run){
        if(wl.ready){
            glVertexAttribPointer(wl.egl.positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), egl_bg_vertices);
            glVertexAttribPointer(wl.egl.texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &egl_bg_vertices[3]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                wl.info.w, wl.info.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, wl.bg);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, egl_indices);

            glVertexAttribPointer(wl.egl.positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), egl_fb_vertices);
            glVertexAttribPointer(wl.egl.texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &egl_fb_vertices[3]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                wl.info.w, wl.info.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, wl.pixels[wl.flip ^ 1]);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, egl_indices);
            eglSwapBuffers(wl.egl.display, wl.egl.surface);
        }
        else{
            usleep(1000);
        }
    }
    return NULL;
}
#endif

static void *video_handler(void *threadid)
{
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

#ifdef QX1000
int fb_init(void)
{
    thread_run = 1;
    wl.bg = SDL_malloc(LCD_W * LCD_H * 2);
    memset(wl.bg, 0, LCD_W * LCD_H * 2);
    pthread_create(&thread_id[0], NULL, wl_thread, NULL);
    pthread_create(&thread_id[1], NULL, draw_thread, NULL);
    while(wl.init == 0){
        usleep(100000);
    }

    wl.flip = 0;
    wl.ready = 0;
    wl.data = SDL_malloc(LCD_W * LCD_H * 2);
    memset(wl.data, 0, LCD_W * LCD_H *2);
    update_wayland_res(640, 480);
    wl.ready = 1;
    return 0;
}

int fb_quit(void)
{
    return 0;
}
#endif

#ifdef TRIMUI
static int ion_alloc(int ion_fd, ion_alloc_info_t* info)
{
    struct ion_allocation_data iad;
    struct ion_fd_data ifd;
    struct ion_custom_data icd;
    sunxi_phys_data spd;

    iad.len = info->size;
    iad.align = sysconf(_SC_PAGESIZE);
    iad.heap_id_mask = ION_HEAP_TYPE_DMA_MASK;
    iad.flags = 0;
    if (ioctl(ion_fd, ION_IOC_ALLOC, &iad) < 0) {
        printf(PREFIX"Failed to call ION_IOC_ALLOC\n");
        return -1;
    }

    icd.cmd = ION_IOC_SUNXI_PHYS_ADDR;
    icd.arg = (uintptr_t)&spd;
    spd.handle = iad.handle;
    if (ioctl(ion_fd, ION_IOC_CUSTOM, &icd) < 0) {
        printf(PREFIX"Failed to call ION_IOC_CUSTOM\n");
        return -1;
    }
    ifd.handle = iad.handle;
    if (ioctl(ion_fd, ION_IOC_MAP, &ifd) < 0) {
        printf(PREFIX"Failed to call ION_IOC_MAP\n");
    }

    info->handle = iad.handle;
    info->fd = ifd.fd;
    info->padd = (void*)spd.phys_addr;
    info->vadd = mmap(0, info->size, PROT_READ | PROT_WRITE, MAP_SHARED, info->fd, 0);
    printf(PREFIX"Mmap padd: 0x%x vadd: 0x%x size: %d\n", (uintptr_t)info->padd, (uintptr_t)info->vadd, info->size);
    return 0;
}

static void ion_free(int ion_fd, ion_alloc_info_t* info)
{
    struct ion_handle_data ihd;

    munmap(info->vadd, info->size);
    close(info->fd);
    ihd.handle = info->handle;
    if (ioctl(ion_fd, ION_IOC_FREE, &ihd) < 0) {
        printf(PREFIX"Failed to call ION_ION_FREE\n");
    }
}

int fb_init(void)
{
    int r = 0;
    uint32_t args[4] = {0, (uintptr_t)&gfx.hw.disp, 1, 0};

    gfx.fb_dev = open("/dev/fb0", O_RDWR);
    gfx.ion_dev = open("/dev/ion", O_RDWR);
    gfx.mem_dev = open("/dev/mem", O_RDWR);
    gfx.disp_dev = open("/dev/disp", O_RDWR);

    if (gfx.fb_dev < 0) {
        printf(PREFIX"Failed to open /dev/fb0\n");
        return -1;
    }

    memset(&gfx.hw.disp, 0, sizeof(disp_layer_config));
    memset(&gfx.hw.buf, 0, sizeof(disp_layer_config));
    gfx.hw.mem = mmap(0, sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE, MAP_SHARED, gfx.mem_dev, OVL_V);

    ioctl(gfx.fb_dev, FBIO_WAITFORVSYNC, &r);

    gfx.hw.disp.channel = DEF_FB_CH;
    gfx.hw.disp.layer_id = DEF_FB_LAYER;
    ioctl(gfx.disp_dev, DISP_LAYER_GET_CONFIG, args);

    gfx.hw.disp.enable = 0;
    ioctl(gfx.disp_dev, DISP_LAYER_SET_CONFIG, args);

    gfx.hw.ion.size = ION_W * ION_H * FB_BPP * 2;
    ion_alloc(gfx.ion_dev, &gfx.hw.ion);

    gfx.hw.buf.channel = SCALER_CH;
    gfx.hw.buf.layer_id = SCALER_LAYER;
    gfx.hw.buf.enable = 1;
    gfx.hw.buf.info.fb.format = DISP_FORMAT_ARGB_8888;
    gfx.hw.buf.info.fb.addr[0] = (uintptr_t)gfx.hw.ion.padd;
    gfx.hw.buf.info.fb.size[0].width = FB_H;
    gfx.hw.buf.info.fb.size[0].height = FB_W;
    gfx.hw.buf.info.mode = LAYER_MODE_BUFFER;
    gfx.hw.buf.info.zorder = SCALER_ZORDER;
    gfx.hw.buf.info.alpha_mode = 0;
    gfx.hw.buf.info.alpha_value = 0;
    gfx.hw.buf.info.screen_win.x = 0;
    gfx.hw.buf.info.screen_win.y = 0;
    gfx.hw.buf.info.screen_win.width  = FB_H;
    gfx.hw.buf.info.screen_win.height = FB_W;
    gfx.hw.buf.info.fb.pre_multiply = 0;
    gfx.hw.buf.info.fb.crop.x = (uint64_t)0 << 32;
    gfx.hw.buf.info.fb.crop.y = (uint64_t)0 << 32;
    gfx.hw.buf.info.fb.crop.width  = (uint64_t)FB_H << 32;
    gfx.hw.buf.info.fb.crop.height = (uint64_t)FB_W << 32;
    args[1] = (uintptr_t)&gfx.hw.buf;
    ioctl(gfx.disp_dev, DISP_LAYER_SET_CONFIG, args);
    ioctl(gfx.fb_dev, FBIO_WAITFORVSYNC, &r);
    return 0;
}

int fb_quit(void)
{
    uint32_t args[4] = {0, (uintptr_t)&gfx.hw.disp, 1, 0};

    gfx.hw.disp.enable = gfx.hw.buf.enable = 0;
    ioctl(gfx.disp_dev, DISP_LAYER_SET_CONFIG, args);

    args[1] = (uintptr_t)&gfx.hw.buf;
    ioctl(gfx.disp_dev, DISP_LAYER_SET_CONFIG, args);

    gfx.hw.disp.enable = 1;
    gfx.hw.disp.channel = DEF_FB_CH;
    gfx.hw.disp.layer_id = DEF_FB_LAYER;
    args[1] = (uintptr_t)&gfx.hw.disp;
    ioctl(gfx.disp_dev, DISP_LAYER_SET_CONFIG, args);

    ion_free(gfx.ion_dev, &gfx.hw.ion);
    munmap(gfx.hw.mem, sysconf(_SC_PAGESIZE));

    close(gfx.fb_dev);
    close(gfx.ion_dev);
    close(gfx.mem_dev);
    close(gfx.disp_dev);

    gfx.fb_dev = -1;
    gfx.ion_dev = -1;
    gfx.mem_dev = -1;
    gfx.disp_dev = -1;
    return 0;
}

void disp_resize(void)
{
    int r = 0;
    uint32_t args[4] = {0, (uintptr_t)&gfx.hw.buf, 1, 0};

    ioctl(gfx.fb_dev, FBIO_WAITFORVSYNC, &r);
    gfx.hw.buf.info.fb.crop.width  = (uint64_t)240 << 32;
    gfx.hw.buf.info.fb.crop.height = (uint64_t)320 << 32;
    ioctl(gfx.disp_dev, DISP_LAYER_SET_CONFIG, args);
    ioctl(gfx.fb_dev, FBIO_WAITFORVSYNC, &r);
}
#endif

#ifdef MMIYOO
int fb_init(void)
{
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
    MI_SYS_Munmap(gfx.fb.virAddr, TMP_SIZE);
    MI_SYS_Munmap(gfx.tmp.virAddr, TMP_SIZE);
    MI_SYS_MMA_Free(gfx.tmp.phyAddr);
    MI_SYS_Munmap(gfx.overlay.virAddr, TMP_SIZE);
    MI_SYS_MMA_Free(gfx.overlay.phyAddr);

    MI_GFX_Close();
    MI_SYS_Exit();
    return 0;
}
#endif

void GFX_Init(void)
{
#ifdef TRIMUI
    int x = 0;
    int y = 0;
    int ox = 32;
    int oy = 24;
    int bx = 16;
    int by = 8;
    uint32_t *dst = NULL;
#endif

    fb_init();
    gfx.thread.pixels = malloc(TMP_SIZE);
    cvt = SDL_CreateRGBSurface(SDL_SWSURFACE, FB_W, FB_H, 32, 0, 0, 0, 0);

#ifdef TRIMUI
    cc = 0;
    for (y = 0; y < 192; y++) {
        for (x = 0; x < 256; x++) {
            dst = (uint32_t *)gfx.hw.ion.vadd;
            LUT_256x192_S10[cc] = (uint32_t)(dst + ((((256 - 1) - x)) * FB_H) + y);
            LUT_256x192_S00_pixel[cc] = (uint32_t)(dst + ((((256 - 1) - x) + ox) * FB_H) + y + oy);
            LUT_256x192_S00_blur[cc] = (uint32_t)(dst + ((((256 - 1) - x) + bx) * FB_H) + y + by);

            dst = (uint32_t *)gfx.hw.ion.vadd + (FB_W * FB_H);
            LUT_256x192_S11[cc] = (uint32_t)(dst + ((((256 - 1) - x)) * FB_H) + y);
            LUT_256x192_S01_pixel[cc] = (uint32_t)(dst + ((((256 - 1) - x) + ox) * FB_H) + y + oy);
            LUT_256x192_S01_blur[cc] = (uint32_t)(dst + ((((256 - 1) - x) + bx) * FB_H) + y + by);
            cc+= 1;
        }
    }
#endif

    is_running = 1;
    gfx.action = GFX_ACTION_NONE;
    pthread_create(&thread, NULL, video_handler, (void *)NULL);
}

void GFX_Quit(void)
{
    void *ret = NULL;

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
#ifdef MMIYOO
    MI_SYS_MemsetPa(gfx.fb.phyAddr, 0, FB_SIZE);
    MI_SYS_MemsetPa(gfx.tmp.phyAddr, 0, TMP_SIZE);
#endif
}

int draw_pen(const void *pixels, int width, int pitch)
{
    int c0 = 0;
    int c1 = 0;
    int w = 28;
    int h = 28;
    int sub = 0;
    int sw = 256;
    int sh = 192;
    int x0 = 0, y0 = 0;
    int x1 = 0, y1 = 0;
    int x = 0, y = 0, is_565 = 0, scale = 1;
    uint16_t r = 0, g = 0, b = 0;
    uint32_t *s = hex_pen;
    uint16_t *d_565 = (uint16_t*)pixels;
    uint32_t *d_888 = (uint32_t*)pixels;

    if ((pitch / width) == 2) {
        is_565 = 1;
    }

    if (width == 512) {
        sw = 512;
        sh = 384;
        scale = 2;
    }

    x = ((evt.mouse.x - evt.mouse.minx) * sw) / (evt.mouse.maxx - evt.mouse.minx);
    y = ((evt.mouse.y - evt.mouse.miny) * sh) / (evt.mouse.maxy - evt.mouse.miny);

    asm ("PLD [%0, #128]"::"r" (s));
    for (c1=0; c1<h; c1++) {
        asm ("PLD [%0, #128]"::"r" (d_565));
        asm ("PLD [%0, #128]"::"r" (d_888));
        for (c0=0; c0<w; c0++) {
            x0 = x1 = (c0 * scale) + x;
            y0 = y1 = (c1 * scale) + (y - sub);

            if (scale == 2) {
                x0+= 1;
                y0+= 1;
            }
            if ((y0 >= 0) && (y0 < sh) && (x0 < sw) && (x0 >= 0)) {
                if (*s) {
                    if (is_565) {
                        r = (*s & 0xf80000) >> 8;
                        g = (*s & 0x00f800) >> 5;
                        b = (*s & 0x0000f8) >> 3;
                        d_565[(y1 * width) + x1] = r | g | b;
                        if (scale == 2) {
                            d_565[((y1 + 0) * width) + (x1 + 1)] = r | g | b;
                            d_565[((y1 + 1) * width) + (x1 + 0)] = r | g | b;
                            d_565[((y1 + 1) * width) + (x1 + 1)] = r | g | b;
                        }
                    }
                    else {
                        d_888[(y1 * width) + x1] = *s;
                        if (scale == 2) {
                            d_888[((y1 + 0) * width) + (x1 + 1)] = r | g | b;
                            d_888[((y1 + 1) * width) + (x1 + 0)] = r | g | b;
                            d_888[((y1 + 1) * width) + (x1 + 1)] = r | g | b;
                        }
                    }
                }
            }
            s+= 1;
        }
    }
    return 0;
}

int GFX_Copy(const void *pixels, SDL_Rect srcrect, SDL_Rect dstrect, int pitch, int alpha, int rotate)
{
#ifdef QX1000
    int x = 0;
    int y = 0;
    const uint32_t *src = pixels;
    uint32_t *dst = (uint32_t *)wl.pixels[wl.flip];

    for (y = 0; y < srcrect.h; y++) {
        for (x = 0; x < srcrect.w; x++) {
            *dst++ = *src++;
        }
        dst+= (FB_W - srcrect.w);
    }
#endif

#ifdef TRIMUI
    int x = 0;
    int y = 0;
    int sw = srcrect.w;
    int sh = srcrect.h;
    uint32_t *dst = NULL;
    uint32_t *src = (uint32_t *)pixels;

    if (pixels == NULL) {
        return -1;
    }

    if ((pitch / srcrect.w) != 4) {
        printf(PREFIX"Only support 32 bits (%dx%dx%d)\n", srcrect.w, srcrect.h, (pitch / srcrect.w));
        return -1;
    }

    dst = (uint32_t *)gfx.hw.ion.vadd + (FB_W * FB_H * gfx.fb.flip);
    for (y = 0; y < sh; y++) {
        for (x = 0; x < sw; x++) {
            dst[(((sw - 1) - x) * FB_H) + y] = *src++;
        }
    }
#endif

#ifdef MMIYOO
    int copy_it = 1;
    MI_U16 u16Fence = 0;
    int is_rgb565 = (pitch / srcrect.w) == 2 ? 1 : 0;

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
#endif

    return 0;
}

void GFX_Flip(void)
{
#ifdef QX1000
    wl.flip ^= 1;
#endif

#ifdef MMIYOO
    MI_SYS_FlushInvCache(gfx.fb.virAddr, FB_SIZE);
    ioctl(gfx.fb_dev, FBIOPAN_DISPLAY, &gfx.vinfo);
    gfx.vinfo.yoffset ^= FB_H;
#endif

#ifdef TRIMUI
    int r = 0;

    gfx.hw.buf.info.fb.addr[0] = (uintptr_t)((uint32_t *)gfx.hw.ion.padd + (FB_W * FB_H * gfx.fb.flip));
    gfx.hw.mem[OVL_V_TOP_LADD0 / 4] = gfx.hw.buf.info.fb.addr[0];
    ioctl(gfx.fb_dev, FBIO_WAITFORVSYNC, &r);
    gfx.fb.flip^= 1;
#endif
}

static int MMIYOO_Available(void)
{
    const char *envr = SDL_getenv("SDL_VIDEODRIVER");
    if((envr) && (SDL_strcmp(envr, MMIYOO_DRIVER_NAME) == 0)) {
        return 1;
    }
    return 0;
}

static void MMIYOO_DeleteDevice(SDL_VideoDevice *device)
{
    SDL_free(device);
}

void MMIYOO_DestroyWindow(_THIS, SDL_Window *window)
{
    GFX_Quit();
}

int MMIYOO_CreateWindow(_THIS, SDL_Window *window)
{
    SDL_SetMouseFocus(window);
    vid.window = window;

#ifdef MMIYOO
    glUpdateBufferSettings(GFX_Flip, &gfx.vinfo.yoffset, gfx.fb.virAddr, (uint8_t*)gfx.fb.virAddr + (FB_W * FB_H * FB_BPP));
#endif

#ifdef QX1000
    update_wayland_res(window->w, window->h);
    glUpdateBufferSettings(GFX_Flip, &wl.flip, wl.pixels[0], wl.pixels[1]);
#endif
    printf(PREFIX"Window %p (width:%d, height:%d)\n", window, window->w, window->h);
    return 0;
}

int MMIYOO_CreateWindowFrom(_THIS, SDL_Window *window, const void *data)
{
    return SDL_Unsupported();
}

static SDL_VideoDevice *MMIYOO_CreateDevice(int devindex)
{
    SDL_VideoDevice *device = NULL;
    SDL_GLDriverData *gldata = NULL;

    if(!MMIYOO_Available()) {
        return (0);
    }

    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if(!device) {
        SDL_OutOfMemory();
        return (0);
    }
    device->is_dummy = SDL_TRUE;

    device->VideoInit = MMIYOO_VideoInit;
    device->VideoQuit = MMIYOO_VideoQuit;
    device->SetDisplayMode = MMIYOO_SetDisplayMode;
    device->PumpEvents = MMIYOO_PumpEvents;
    device->CreateSDLWindow = MMIYOO_CreateWindow;
    device->CreateSDLWindowFrom = MMIYOO_CreateWindowFrom;
    device->CreateWindowFramebuffer = MMIYOO_CreateWindowFramebuffer;
    device->UpdateWindowFramebuffer = MMIYOO_UpdateWindowFramebuffer;
    device->DestroyWindowFramebuffer = MMIYOO_DestroyWindowFramebuffer;
    device->DestroyWindow = MMIYOO_DestroyWindow;

    device->GL_LoadLibrary = glLoadLibrary;
    device->GL_GetProcAddress = glGetProcAddress;
    device->GL_CreateContext = glCreateContext;
    device->GL_SetSwapInterval = glSetSwapInterval;
    device->GL_SwapWindow = glSwapWindow;
    device->GL_MakeCurrent = glMakeCurrent;
    device->GL_DeleteContext = glDeleteContext;
    device->GL_UnloadLibrary = glUnloadLibrary;
    
    gldata = (SDL_GLDriverData*)SDL_calloc(1, sizeof(SDL_GLDriverData));
    if(gldata == NULL) {
        SDL_OutOfMemory();
        SDL_free(device);
        return NULL;
    }

    device->gl_data = gldata;
    device->free = MMIYOO_DeleteDevice;
    return device;
}

VideoBootStrap MMIYOO_bootstrap = {MMIYOO_DRIVER_NAME, "MMIYOO VIDEO DRIVER", MMIYOO_CreateDevice};

int MMIYOO_VideoInit(_THIS)
{
#ifdef MMIYOO
    FILE *fd = NULL;
    char buf[MAX_PATH] = {0}; 
#endif

    SDL_DisplayMode mode = {0};
    SDL_VideoDisplay display = {0};

    printf(PREFIX"%s\n", __func__);
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
#ifdef MMIYOO
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
#endif

    GFX_Init();
    MMIYOO_EventInit();
    return 0;
}

static int MMIYOO_SetDisplayMode(_THIS, SDL_VideoDisplay *display, SDL_DisplayMode *mode)
{
    return 0;
}

void MMIYOO_VideoQuit(_THIS)
{
    printf(PREFIX"%s\n", __func__);
    MMIYOO_EventDeinit();
}

#endif

