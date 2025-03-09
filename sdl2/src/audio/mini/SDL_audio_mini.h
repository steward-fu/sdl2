// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#ifndef __SDL_AUDIO_MINI_H__
#define __SDL_AUDIO_MINI_H__

#include "../../SDL_internal.h"
#include "../SDL_sysaudio.h"

#define _THIS SDL_AudioDevice *this

#define MINI_DRIVER_NAME            "mini"
#define PREFIX                      "[SND] "
#define MI_AUDIO_SAMPLE_PER_FRAME   768
#define FUDGE_TICKS                 10
#define FREQ                        44100
#define CHANNELS                    2

struct SDL_PrivateAudioData {
    int audio_fd;
    uint8_t *mixbuf;
    int mixlen;
};

#endif

