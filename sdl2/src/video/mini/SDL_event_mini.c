// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_MINI

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <dirent.h>
#include <linux/input.h>

#include "../../events/SDL_events_c.h"
#include "../../core/linux/SDL_evdev.h"
#include "../../thread/SDL_systhread.h"

#include "SDL_video_mini.h"
#include "SDL_event_mini.h"

static int running = 0;
static SDL_Thread *thread = NULL;

uint8_t mykey[KEY_MAX][2] = { 0 };
static SDL_Scancode mymap[KEY_MAX] = { 0 };

int Mini_InputHandler(void *data)
{
    int fd = -1;
    struct input_event ev = {0};

    debug("%s++\n", __func__);
    fd = open("/dev/input/event0", O_RDONLY);
    if (fd < 0){
        debug("%s, failed to open input device\n", __func__);
        return 0;
    }

    running = 1;
    fcntl(fd, F_SETFL, O_NONBLOCK);
    while (running) {
        if (read(fd, &ev, sizeof(struct input_event)) > 0) {
            if (ev.type == EV_KEY) {
                mykey[ev.code][ev.value] = 1;
                debug("%s, code:%d, value:%d\n", __func__, ev.code, ev.value);
            }
        }
        usleep(1000);
    }
    
    if (fd > 0) {
        close(fd);
        fd = -1;
    }
    debug("%s--\n", __func__);
    return 0;
}

void Mini_EventInit(void)
{
    debug("%s\n", __func__);

     mymap[KEY_0] = SDLK_0;
     mymap[KEY_1] = SDLK_1;
     mymap[KEY_2] = SDLK_2;
     mymap[KEY_3] = SDLK_3;
     mymap[KEY_4] = SDLK_4;
     mymap[KEY_5] = SDLK_5;
     mymap[KEY_6] = SDLK_6;
     mymap[KEY_7] = SDLK_7;
     mymap[KEY_8] = SDLK_8;
     mymap[KEY_9] = SDLK_9;

     mymap[KEY_A] = SDLK_a;
     mymap[KEY_B] = SDLK_b;
     mymap[KEY_C] = SDLK_c;
     mymap[KEY_D] = SDLK_d;
     mymap[KEY_E] = SDLK_e;
     mymap[KEY_F] = SDLK_f;
     mymap[KEY_G] = SDLK_g;
     mymap[KEY_H] = SDLK_h;
     mymap[KEY_I] = SDLK_i;
     mymap[KEY_J] = SDLK_j;
     mymap[KEY_K] = SDLK_k;
     mymap[KEY_L] = SDLK_l;
     mymap[KEY_M] = SDLK_m;
     mymap[KEY_N] = SDLK_n;
     mymap[KEY_O] = SDLK_o;
     mymap[KEY_P] = SDLK_p;
     mymap[KEY_Q] = SDLK_q;
     mymap[KEY_R] = SDLK_r;
     mymap[KEY_S] = SDLK_s;
     mymap[KEY_T] = SDLK_t;
     mymap[KEY_U] = SDLK_u;
     mymap[KEY_V] = SDLK_v;
     mymap[KEY_W] = SDLK_w;
     mymap[KEY_X] = SDLK_x;
     mymap[KEY_Y] = SDLK_y;
     mymap[KEY_Z] = SDLK_z;

     mymap[KEY_UP] = SDLK_UP;
     mymap[KEY_DOWN] = SDLK_DOWN;
     mymap[KEY_LEFT] = SDLK_LEFT;
     mymap[KEY_RIGHT] = SDLK_RIGHT;
     
     mymap[KEY_ESC] = SDLK_ESCAPE;
     mymap[KEY_SPACE] = SDLK_SPACE;
     mymap[KEY_CAPSLOCK] = SDLK_CAPSLOCK;
     mymap[KEY_BACKSPACE] = SDLK_BACKSPACE;
     mymap[KEY_TAB] = SDLK_TAB;
     mymap[KEY_GRAVE] = SDLK_BACKQUOTE;
     mymap[KEY_COMMA] = SDLK_COMMA;
     mymap[KEY_DOT] = SDLK_PERIOD;
     mymap[KEY_APOSTROPHE] = SDLK_QUOTEDBL;
     mymap[KEY_LEFTBRACE] = SDLK_LEFTBRACKET;
     mymap[KEY_RIGHTBRACE] = SDLK_RIGHTBRACKET;
     mymap[KEY_ENTER] = SDLK_RETURN;
     mymap[KEY_MINUS] = SDLK_MINUS;
     mymap[KEY_EQUAL] = SDLK_EQUALS;
     mymap[KEY_SLASH] = SDLK_SLASH;
     mymap[KEY_BACKSLASH] = SDLK_BACKSLASH;

     mymap[KEY_F1] = SDLK_F1;
     mymap[KEY_F2] = SDLK_F2;
     mymap[KEY_F3] = SDLK_F3;
     mymap[KEY_F4] = SDLK_F4;
     mymap[KEY_F5] = SDLK_F5;

    mymap[KEY_RIGHTSHIFT] = SDLK_RSHIFT;
    mymap[KEY_LEFTSHIFT] = SDLK_LSHIFT;
    mymap[KEY_RIGHTCTRL] = SDLK_RCTRL;
    mymap[KEY_LEFTCTRL] = SDLK_LCTRL;
    mymap[KEY_RIGHTALT] = SDLK_RALT;
    mymap[KEY_LEFTALT] = SDLK_LALT;

    if ((thread = SDL_CreateThreadInternal(Mini_InputHandler, "Mini_InputHandler", 4096, NULL)) == NULL) {
        debug("%s, failed to create input thread\n", __func__);
    }
}

void Mini_EventQuit(void)
{
    debug("%s\n", __func__);
    running = 0;
    SDL_WaitThread(thread, NULL);
}

void Mini_PumpEvents(_THIS)
{
    int c0 = 0;

    for (c0 = 0; c0 < KEY_MAX; c0++) {
        if (mykey[c0][1] && mymap[c0]) {
            mykey[c0][1] = 0;
            debug("%s, key pressed: %d\n", __func__, mymap[c0]);
            SDL_SendKeyboardKey(SDL_PRESSED, SDL_GetScancodeFromKey(mymap[c0]));
        }
        if (mykey[c0][0] && mymap[c0]) {
            mykey[c0][0] = 0;
            debug("%s, key released: %d\n", __func__, mymap[c0]);
            SDL_SendKeyboardKey(SDL_RELEASED, SDL_GetScancodeFromKey(mymap[c0]));
        }
    }
}

#endif

