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

#include "SDL_video_mmiyoo.h"
#include "SDL_event_mmiyoo.h"

#ifdef MMIYOO
    #define UP      103
    #define DOWN    108
    #define LEFT    105
    #define RIGHT   106
    #define A       57
    #define B       29
    #define X       42
    #define Y       56
    #define L1      18
    #define L2      15
    #define R1      20
    #define R2      14
    #define START   28
    #define SELECT  97
    #define MENU    1
    #define POWER   116
    #define VOLUP   115
    #define VOLDOWN 114
#endif

#ifdef TRIMUI
    #define UP      103
    #define DOWN    108
    #define LEFT    105
    #define RIGHT   106
    #define A       57
    #define B       29
    #define X       42
    #define Y       56
    #define L1      15
    #define R1      14
    #define START   28
    #define SELECT  97
    #define MENU    1
#endif

#ifdef FUNKEYS
    #define UP      22
    #define DOWN    32
    #define LEFT    38
    #define RIGHT   19
    #define A       30
    #define B       48
    #define X       45
    #define Y       21
    #define L1      50
    #define R1      49
    #define START   31
    #define SELECT  37
    #define MENU    16
#endif

#ifdef PANDORA
    #define UP      103
    #define DOWN    108
    #define LEFT    105
    #define RIGHT   106
    #define A       107
    #define B       109
    #define X       104
    #define Y       102
    #define L1      54
    #define R1      97
    #define START   56
    #define SELECT  29
    #define MENU    139
#endif

#ifdef QX1000
    #define UP      16 // 'Q'
    #define DOWN    30 // 'A'
    #define LEFT    43 // '\'
    #define RIGHT   31 // 'S'
    #define A       38 // 'L'
    #define B       37 // 'K'
    #define X       25 // 'P'
    #define Y       24 // 'O'
    #define R1      17 // 'W'
    #define R10     51 // ','
    #define R2      28 // 'Enter'
    #define L1      41 // '`'
    #define L10     50 // 'M'
    #define L2      15 // 'Tab'
    #define SELECT  53 // '/'
    #define START   29 // 'RCTRL'
    #define MENU    57 // ' '
    #define QSAVE   46 // 'C'
    #define QLOAD   49 // 'N'
    #define EXIT    1  // 'ESC'
#endif

MMIYOO_EventInfo evt = {0};

extern GFX gfx;
extern MMIYOO_VideoInfo vid;

static int running = 0;
static int event_fd = -1;
static int lower_speed = 0;
static SDL_sem *event_sem = NULL;
static SDL_Thread *thread = NULL;
static uint32_t pre_ticks = 0;
static uint32_t pre_keypad_bitmaps = 0;

extern int FB_W;
extern int FB_H;

#ifdef TRIMUI
typedef struct _cust_key_t {
    int fd;
    uint8_t *mem;
    uint32_t *gpio;
    uint32_t pre_cfg;
} cust_key_t;

static cust_key_t cust_key = {0};
#endif

const SDL_Scancode code[]={
    SDLK_UP,            // UP
    SDLK_DOWN,          // DOWN
    SDLK_LEFT,          // LEFT
    SDLK_RIGHT,         // RIGHT
    SDLK_LCTRL,         // A
    SDLK_LALT,          // B
    SDLK_LSHIFT,        // X
    SDLK_SPACE,         // Y
    SDLK_TAB,           // L1
    SDLK_BACKSPACE,     // R1
    SDLK_PAGEDOWN,      // L2
    SDLK_PAGEUP,        // R2
    SDLK_ESCAPE,        // SELECT
    SDLK_RETURN,        // START
    SDLK_HOME,          // MENU
    SDLK_0,             // QUICK SAVE
    SDLK_1,             // QUICK LOAD
    SDLK_2,             // FAST FORWARD
    SDLK_3,             // EXIT
    SDLK_HOME,          // MENU (Onion system)
};

int volume_inc(void);
int volume_dec(void);

static void check_mouse_pos(void)
{
    if (evt.mouse.y < evt.mouse.miny) {
        evt.mouse.y = evt.mouse.miny;
    }
    if (evt.mouse.y > evt.mouse.maxy) {
        evt.mouse.y = evt.mouse.maxy;
    }
    if (evt.mouse.x < evt.mouse.minx) {
        evt.mouse.x = evt.mouse.minx;
    }
    if (evt.mouse.x >= evt.mouse.maxx) {
        evt.mouse.x = evt.mouse.maxx;
    }
}

static int get_move_interval(int type)
{
    float move = 0.0;
    long yv = 35000;
    long xv = 30000;

    if (lower_speed) {
        yv*= 2;
        xv*= 2;
    }

    move = ((float)clock() - pre_ticks) / ((type == 0) ? xv : yv);
    if (move <= 0.0) {
        move = 1.0;
    }
    return (int)(1.0 * move);
}

static void set_key(uint32_t bit, int val)
{
    if (val) {
        evt.keypad.bitmaps|= (1 << bit);
    }
    else {
        evt.keypad.bitmaps&= ~(1 << bit);
    }
}

int EventUpdate(void *data)
{
    struct input_event ev = {0};

    uint32_t l1 = L1;
    uint32_t r1 = R1;
#ifdef MMIYOO
    uint32_t l2 = L2;
    uint32_t r2 = R2;
#endif

    uint32_t a = A;
    uint32_t b = B;
    uint32_t x = X;
    uint32_t y = Y;

    uint32_t up = UP;
    uint32_t down = DOWN;
    uint32_t left = LEFT;
    uint32_t right = RIGHT;

    while (running) {
        SDL_SemWait(event_sem);

        up = UP;
        down = DOWN;
        left = LEFT;
        right = RIGHT;

        a = A;
        b = B;
        x = X;
        y = Y;

#ifdef MMIYOO
        l1 = L1;
        l2 = L2;
        r1 = R1;
        r2 = R2;
#endif

#ifdef TRIMUI
        if (cust_key.gpio != NULL) {
            static uint32_t pre_value = 0;
            uint32_t v = *cust_key.gpio & 0x800;

            if (v != pre_value) {
                pre_value = v;
                set_key(MYKEY_R2, !v);
            }
        }
#endif

        if (event_fd > 0) {
            if (read(event_fd, &ev, sizeof(struct input_event))) {
                if ((ev.type == EV_KEY) && (ev.value != 2)) {
                    if (ev.code == l1)      { set_key(MYKEY_L1,    ev.value); }
                    if (ev.code == r1)      { set_key(MYKEY_R1,    ev.value); }
#ifdef MMIYOO
                    if (ev.code == l2)      { set_key(MYKEY_L2,    ev.value); }
                    if (ev.code == r2)      { set_key(MYKEY_R2,    ev.value); }
#endif
                    if (ev.code == up)      { set_key(MYKEY_UP,    ev.value); }
                    if (ev.code == down)    { set_key(MYKEY_DOWN,  ev.value); }
                    if (ev.code == left)    { set_key(MYKEY_LEFT,  ev.value); }
                    if (ev.code == right)   { set_key(MYKEY_RIGHT, ev.value); }
                    if (ev.code == a)       { set_key(MYKEY_A,     ev.value); }
                    if (ev.code == b)       { set_key(MYKEY_B,     ev.value); }
                    if (ev.code == x)       { set_key(MYKEY_X,     ev.value); }
                    if (ev.code == y)       { set_key(MYKEY_Y,     ev.value); }

                    switch (ev.code) {
                    case START:   set_key(MYKEY_START, ev.value);   break;
                    case SELECT:  set_key(MYKEY_SELECT, ev.value);  break;
                    case MENU:    set_key(MYKEY_MENU, ev.value);    break;
#ifdef MMIYOO
                    case POWER:   set_key(MYKEY_POWER, ev.value);   break;
                    case VOLUP:   set_key(MYKEY_VOLUP, ev.value);   break;
                    case VOLDOWN: set_key(MYKEY_VOLDOWN, ev.value); break;
#endif
                    }
                }

                if (!(evt.keypad.bitmaps & 0x0f)) {
                    pre_ticks = clock();
                }
            }
        }
        SDL_SemPost(event_sem);
        usleep(1000000 / 60);
    }
    
    return 0;
}

void MMIYOO_EventInit(void)
{
    pre_keypad_bitmaps = 0;
    memset(&evt, 0, sizeof(evt));
    evt.mouse.minx = 0;
    evt.mouse.miny = 0;
    evt.mouse.maxx = FB_W;
    evt.mouse.maxy = FB_H;
    evt.mouse.x = (evt.mouse.maxx - evt.mouse.minx) / 2;
    evt.mouse.y = (evt.mouse.maxy - evt.mouse.miny) / 2;

#ifdef TRIMUI
    cust_key.gpio = NULL;
    cust_key.fd = open("/dev/mem", O_RDWR);
    printf(PREFIX"cust_key.fd %d\n", cust_key.fd);
    if (cust_key.fd > 0) {
        cust_key.mem = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, cust_key.fd, 0x01c20000);
        printf(PREFIX"cust_key.mem %p\n", cust_key.mem);
        if (cust_key.mem != MAP_FAILED) {
            uint32_t *p = NULL;

            p = (uint32_t *)(cust_key.mem + 0x800 + (0x24 * 6) + 0x04);
            cust_key.pre_cfg = *p;
            *p &= 0xffff0fff;

            p = (uint32_t *)(cust_key.mem + 0x800 + (0x24 * 6) + 0x1c);
            *p |= 0x00400000;

            cust_key.gpio = (uint32_t *)(cust_key.mem + 0x800 + (0x24 * 6) + 0x10);
            printf(PREFIX"cust_key.gpio %p\n", cust_key.gpio);
        }
    }
    evt.mouse.y = (evt.mouse.maxy - evt.mouse.miny) / 2;
#endif
    evt.mode = MMIYOO_KEYPAD_MODE;

    event_fd = open("/dev/input/event0", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if(event_fd < 0){
        printf(PREFIX"failed to open /dev/input/event0\n");
    }

    if((event_sem =  SDL_CreateSemaphore(1)) == NULL) {
        SDL_SetError("Can't create input semaphore");
        return;
    }

    running = 1;
    if((thread = SDL_CreateThreadInternal(EventUpdate, "MMIYOOInputThread", 4096, NULL)) == NULL) {
        SDL_SetError("Can't create input thread");
        return;
    }
}

void MMIYOO_EventDeinit(void)
{
    running = 0;
    SDL_WaitThread(thread, NULL);
    SDL_DestroySemaphore(event_sem);
    if(event_fd > 0) {
        close(event_fd);
        event_fd = -1;
    }

#ifdef TRIMUI
    if (cust_key.fd > 0) {
        uint32_t *p = NULL;

        p = (uint32_t *)(cust_key.mem + 0x800 + (0x24 * 6) + 0x04);
        *p = cust_key.pre_cfg;
        munmap(cust_key.mem, 4096);
        close(cust_key.fd);

        cust_key.gpio = NULL;
        cust_key.fd = -1;
    }
#endif
}

void MMIYOO_PumpEvents(_THIS)
{
    SDL_SemWait(event_sem);
    if (evt.mode == MMIYOO_KEYPAD_MODE) {
        if (pre_keypad_bitmaps != evt.keypad.bitmaps) {
            int cc = 0;
            uint32_t bit = 0;
            uint32_t changed = pre_keypad_bitmaps ^ evt.keypad.bitmaps;

            for (cc=0; cc<=MYKEY_LAST_BITS; cc++) {
                bit = 1 << cc;
                if (changed & bit) {
                    SDL_SendKeyboardKey((evt.keypad.bitmaps & bit) ? SDL_PRESSED : SDL_RELEASED, SDL_GetScancodeFromKey(code[cc]));
                }
            }

#ifdef TRIMUI
            if (pre_keypad_bitmaps & (1 << MYKEY_R2)) {
                set_key(MYKEY_R2, 0);
            }
            if (pre_keypad_bitmaps & (1 << MYKEY_L2)) {
                set_key(MYKEY_L2, 0);
            }
#endif
            pre_keypad_bitmaps = evt.keypad.bitmaps;
        }
    }
    else {
        int updated = 0;
        if (pre_keypad_bitmaps != evt.keypad.bitmaps) {
            uint32_t cc = 0;
            uint32_t bit = 0;
            uint32_t changed = pre_keypad_bitmaps ^ evt.keypad.bitmaps;

            if (changed & (1 << MYKEY_A)) {
                SDL_SendMouseButton(vid.window, 0, (evt.keypad.bitmaps & (1 << MYKEY_A)) ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_LEFT);
            }

            for (cc=0; cc<=MYKEY_LAST_BITS; cc++) {
                bit = 1 << cc;
                if ((cc == MYKEY_FF) || (cc == MYKEY_QSAVE) || (cc == MYKEY_QLOAD) || (cc == MYKEY_EXIT) || (cc == MYKEY_R2)) {
                    if (changed & bit) {
                        SDL_SendKeyboardKey((evt.keypad.bitmaps & bit) ? SDL_PRESSED : SDL_RELEASED, SDL_GetScancodeFromKey(code[cc]));
                    }
                }
                if (cc == MYKEY_R1) {
                    if (changed & bit) {
                        lower_speed = (evt.keypad.bitmaps & bit);
                    }
                }
            }
        }

        if (evt.keypad.bitmaps & (1 << MYKEY_UP)) {
            updated = 1;
            evt.mouse.y-= get_move_interval(1);
        }
        if (evt.keypad.bitmaps & (1 << MYKEY_DOWN)) {
            updated = 1;
            evt.mouse.y+= get_move_interval(1);
        }
        if (evt.keypad.bitmaps & (1 << MYKEY_LEFT)) {
            updated = 1;
            evt.mouse.x-= get_move_interval(0);
        }
        if (evt.keypad.bitmaps & (1 << MYKEY_RIGHT)) {
            updated = 1;
            evt.mouse.x+= get_move_interval(0);
        }
        check_mouse_pos();

        if(updated){
            SDL_SendMouseMotion(vid.window, 0, 0, evt.mouse.x, evt.mouse.y);
        }

#ifdef TRIMUI
            if (pre_keypad_bitmaps & (1 << MYKEY_R2)) {
                set_key(MYKEY_R2, 0);
            }
            if (pre_keypad_bitmaps & (1 << MYKEY_L2)) {
                set_key(MYKEY_L2, 0);
            }
#endif
        pre_keypad_bitmaps = evt.keypad.bitmaps;
    }
    SDL_SemPost(event_sem);
}

#endif

