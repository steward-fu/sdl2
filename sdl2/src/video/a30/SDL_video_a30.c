/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

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

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "../../SDL_internal.h"
#include "SDL_version.h"
#include "SDL_syswm.h"
#include "SDL_loadso.h"
#include "SDL_events.h"
#include "../SDL_sysvideo.h"
#include "../../events/SDL_events_c.h"

#include "SDL_event_a30.h"
#include "SDL_video_a30.h"

struct _video vid = {0};

GLfloat bgVertices[] = {
   -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,
   -1.0f, -1.0f,  0.0f,  0.0f,  1.0f,
    1.0f, -1.0f,  0.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  0.0f,  1.0f,  0.0f
};

GLfloat vVertices[] = {
   -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,
   -1.0f, -1.0f,  0.0f,  0.0f,  1.0f,
    1.0f, -1.0f,  0.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  0.0f,  1.0f,  0.0f
};
GLushort indices[] = {0, 1, 2, 0, 2, 3};

const char *vShaderSrc =
    "attribute vec4 a_position;   \n"
    "attribute vec2 a_texCoord;   \n"
    "attribute float a_alpha;     \n"
    "varying vec2 v_texCoord;     \n"
    "void main()                  \n"
    "{                            \n"
    "    const float angle = 90.0 * (3.1415 * 2.0) / 360.0;                                                                            \n"
    "    mat4 rot = mat4(cos(angle), -sin(angle), 0.0, 0.0, sin(angle), cos(angle), 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0); \n"
    "    gl_Position = a_position * rot; \n"
    "    v_texCoord = a_texCoord;        \n"
    "}                                   \n";

const char *fShaderSrc =
    "precision mediump float;                                  \n"
    "varying vec2 v_texCoord;                                  \n"
    "uniform float s_alpha;                                    \n"
    "uniform sampler2D s_texture;                              \n"
    "void main()                                               \n"
    "{                                                         \n"
    "    if (s_alpha >= 2.0) {                                 \n"
    "        gl_FragColor = texture2D(s_texture, v_texCoord);  \n"
    "    }                                                     \n"
    "    else if (s_alpha > 0.0) {                             \n"
    "        vec3 tex = texture2D(s_texture, v_texCoord).rgb;  \n"
    "        gl_FragColor = vec4(tex, s_alpha);                \n"
    "    }                                                     \n"
    "    else {                                                \n"
    "        vec3 tex = texture2D(s_texture, v_texCoord).rgb;  \n"
    "        gl_FragColor = vec4(tex, 1.0);                    \n"
    "    }                                                     \n"
    "}                                                         \n";

static struct _cpu_clock cpu_clock[] = {
    {96, 0x80000110},
    {144, 0x80000120},
    {192, 0x80000130},
    {216, 0x80000220},
    {240, 0x80000410},
    {288, 0x80000230},
    {336, 0x80000610},
    {360, 0x80000420},
    {384, 0x80000330},
    {432, 0x80000520},
    {480, 0x80000430},
    {504, 0x80000620},
    {528, 0x80000a10},
    {576, 0x80000530},
    {624, 0x80000c10},
    {648, 0x80000820},
    {672, 0x80000630},
    {720, 0x80000920},
    {768, 0x80000730},
    {792, 0x80000a20},
    {816, 0x80001010},
    {864, 0x80000830},
    {864, 0x80001110},
    {912, 0x80001210},
    {936, 0x80000c20},
    {960, 0x80000930},
    {1008, 0x80000d20},
    {1056, 0x80000a30},
    {1080, 0x80000e20},
    {1104, 0x80001610},
    {1152, 0x80000b30},
    {1200, 0x80001810},
    {1224, 0x80001020},
    {1248, 0x80000c30},
    {1296, 0x80001120},
    {1344, 0x80000d30},
    {1368, 0x80001220},
    {1392, 0x80001c10},
    {1440, 0x80000e30},
    {1488, 0x80001e10},
    {1512, 0x80001420},
    {1536, 0x80000f30},
    {1584, 0x80001520},
    {1632, 0x80001030},
    {1656, 0x80001620},
    {1728, 0x80001130},
    {1800, 0x80001820},
    {1824, 0x80001230},
    {1872, 0x80001920},
    {1920, 0x80001330},
    {1944, 0x80001a20},
    {2016, 0x80001430},
    {2088, 0x80001c20},
    {2112, 0x80001530},
    {2160, 0x80001d20},
    {2208, 0x80001630},
    {2232, 0x80001e20},
    {2304, 0x80001730},
    {2400, 0x80001830},
    {2496, 0x80001930},
    {2592, 0x80001a30},
    {2688, 0x80001b30},
    {2784, 0x80001c30},
    {2880, 0x80001d30},
    {2976, 0x80001e30},
    {3072, 0x80001f30},
};

static int max_cpu_item = sizeof(cpu_clock) / sizeof(struct _cpu_clock);

static int get_core(int index)
{
    FILE *fd = NULL;
    char buf[255] = {0};

    sprintf(buf, "cat /sys/devices/system/cpu/cpu%d/online", index % 4);
    fd = popen(buf, "r");
    if (fd == NULL) {
        return -1;
    }
    fgets(buf, sizeof(buf), fd);
    pclose(fd);
    return atoi(buf);
}

static void check_before_set(int num, int v)
{
    char buf[255] = {0};

    if (get_core(num) != v) {
        sprintf(buf, "echo %d > /sys/devices/system/cpu/cpu%d/online", v ? 1 : 0, num);
        system(buf);
    }
}

static void set_core(int n)
{
    if (n <= 1) {
        printf(PREFIX"New CPU Core: 1\n");
        check_before_set(0, 1);
        check_before_set(1, 0);
        check_before_set(2, 0);
        check_before_set(3, 0);
    }
    else if (n == 2) {
        printf(PREFIX"New CPU Core: 2\n");
        check_before_set(0, 1);
        check_before_set(1, 1);
        check_before_set(2, 0);
        check_before_set(3, 0);
    }
    else if (n == 3) {
        printf(PREFIX"New CPU Core: 3\n");
        check_before_set(0, 1);
        check_before_set(1, 1);
        check_before_set(2, 1);
        check_before_set(3, 0);
    }
    else {
        printf(PREFIX"New CPU Core: 4\n");
        check_before_set(0, 1);
        check_before_set(1, 1);
        check_before_set(2, 1);
        check_before_set(3, 1);
    }
}

static int set_best_match_cpu_clock(int clk)
{
    int cc = 0;

    system("echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
    for (cc = 0; cc < max_cpu_item; cc++) {
        if (cpu_clock[cc].clk >= clk) {
            printf(PREFIX"Set Best Match CPU %dMHz (0x%08x)\n", cpu_clock[cc].clk, cpu_clock[cc].reg);
            *vid.cpu_ptr = cpu_clock[cc].reg;
            return cc;
        }
    }
    return -1;
}

static void A30_Destroy(SDL_VideoDevice *device)
{
    printf(PREFIX"%s\n", __func__);
    if (device->driverdata != NULL) {
        SDL_free(device->driverdata);
        device->driverdata = NULL;
    }
}

int A30_VideoInit(_THIS)
{
    SDL_DisplayMode mode = {0};
    SDL_VideoDisplay display = {0};

    EGLint egl_major = 0;
    EGLint egl_minor = 0;
    EGLint num_configs = 0;
    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE,   8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE,  8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    EGLint window_attributes[] = {
        EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
        EGL_NONE
    };
    EGLint const context_attributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE,
    };

    printf(PREFIX"%s\n", __func__);
    vid.mem_fd = open("/dev/mem", O_RDWR);
    if (vid.mem_fd < 0) {
        printf("Failed to open /dev/mem\n");
        return -1;
    }

    vid.ccu_mem = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, vid.mem_fd, CCU_BASE);
    if (vid.ccu_mem == MAP_FAILED) {
        printf("Failed to map memory\n");
        return -1;
    }

    printf(PREFIX"CCU MMap %p\n", vid.ccu_mem);
    vid.cpu_ptr = (uint32_t *)&vid.ccu_mem[0x00];

    set_best_match_cpu_clock(INIT_CPU_CLOCK);
    set_core(INIT_CPU_CORE);

    vid.eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(vid.eglDisplay, &egl_major, &egl_minor);
    eglChooseConfig(vid.eglDisplay, config_attribs, &vid.eglConfig, 1, &num_configs);
    vid.eglSurface = eglCreateWindowSurface(vid.eglDisplay, vid.eglConfig, 0, window_attributes);
    vid.eglContext = eglCreateContext(vid.eglDisplay, vid.eglConfig, EGL_NO_CONTEXT, context_attributes);
    eglMakeCurrent(vid.eglDisplay, vid.eglSurface, vid.eglSurface, vid.eglContext);

    vid.vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vid.vShader, 1, &vShaderSrc, NULL);
    glCompileShader(vid.vShader);

    vid.fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vid.fShader, 1, &fShaderSrc, NULL);
    glCompileShader(vid.fShader);

    vid.pObject = glCreateProgram();
    glAttachShader(vid.pObject, vid.vShader);
    glAttachShader(vid.pObject, vid.fShader);
    glLinkProgram(vid.pObject);
    glUseProgram(vid.pObject);

    eglSwapInterval(vid.eglDisplay, 1);
    vid.posLoc = glGetAttribLocation(vid.pObject, "a_position");
    vid.texLoc = glGetAttribLocation(vid.pObject, "a_texCoord");
    vid.samLoc = glGetUniformLocation(vid.pObject, "s_texture");
    vid.alphaLoc = glGetUniformLocation(vid.pObject, "s_alpha");

    glGenTextures(TEX_MAX, vid.texID);

    glViewport(0, 0, LCD_H, LCD_W);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnableVertexAttribArray(vid.posLoc);
    glEnableVertexAttribArray(vid.texLoc);
    glUniform1i(vid.samLoc, 0);
    glUniform1f(vid.alphaLoc, 0.0);

    vid.fb_mem[0] = malloc(LCD_W * LCD_H * 4);
    vid.fb_mem[1] = malloc(LCD_W * LCD_H * 4);

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

    SDL_AddVideoDisplay(&display, SDL_FALSE);

    A30_EventInit();
    return 0;
}

void A30_VideoQuit(_THIS)
{
    printf(PREFIX"%s\n", __func__);
}

void A30_GetDisplayModes(_THIS, SDL_VideoDisplay *display)
{
    printf(PREFIX"%s\n", __func__);
    SDL_AddDisplayMode(display, &display->current_mode);
}

int A30_SetDisplayMode(_THIS, SDL_VideoDisplay *display, SDL_DisplayMode *mode)
{
    printf(PREFIX"%s\n", __func__);
    return 0;
}

int A30_CreateWindow(_THIS, SDL_Window *window)
{
    vid.win = window;
    SDL_SetMouseFocus(window);
    printf(PREFIX"Window %p (width:%d, height:%d)\n", window, window->w, window->h);
    return 0;
}

void A30_DestroyWindow(_THIS, SDL_Window *window)
{
    printf(PREFIX"%s\n", __func__);
}

void A30_SetWindowTitle(_THIS, SDL_Window *window)
{
    printf(PREFIX"%s\n", __func__);
}

void A30_SetWindowPosition(_THIS, SDL_Window *window)
{
    printf(PREFIX"%s\n", __func__);
}

void A30_SetWindowSize(_THIS, SDL_Window *window)
{
    printf(PREFIX"%s\n", __func__);
}

void A30_ShowWindow(_THIS, SDL_Window *window)
{
    printf(PREFIX"%s\n", __func__);
}

void A30_HideWindow(_THIS, SDL_Window *window)
{
    printf(PREFIX"%s\n", __func__);
}

SDL_bool A30_GetWindowWMInfo(_THIS, SDL_Window *window, struct SDL_SysWMinfo *info)
{
    printf(PREFIX"%s\n", __func__);
    return SDL_TRUE;
}

static SDL_VideoDevice* A30_CreateDevice(int devindex)
{
    SDL_VideoDevice *device = NULL;

    printf(PREFIX"%s\n", __func__);
    device = (SDL_VideoDevice *)SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (device == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }

    device->driverdata = NULL;
    device->num_displays = 0;
    device->free = A30_Destroy;
    device->VideoInit = A30_VideoInit;
    device->VideoQuit = A30_VideoQuit;
    device->GetDisplayModes = A30_GetDisplayModes;
    device->SetDisplayMode = A30_SetDisplayMode;
    device->CreateSDLWindow = A30_CreateWindow;
    device->SetWindowTitle = A30_SetWindowTitle;
    device->SetWindowPosition = A30_SetWindowPosition;
    device->SetWindowSize = A30_SetWindowSize;
    device->ShowWindow = A30_ShowWindow;
    device->HideWindow = A30_HideWindow;
    device->DestroyWindow = A30_DestroyWindow;
    device->GetWindowWMInfo = A30_GetWindowWMInfo;
    device->PumpEvents = A30_PumpEvents;
    return device;
}

VideoBootStrap A30_bootstrap = { "a30", "Miyoo A30 Video Driver", A30_CreateDevice };

