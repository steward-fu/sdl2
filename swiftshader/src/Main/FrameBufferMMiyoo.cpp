// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
// Copyright 2022 Steward Fu <steward.fu@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "FrameBufferMMiyoo.hpp"

#include "Common/Timer.hpp"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

namespace sw {
	FrameBufferMMiyoo::FrameBufferMMiyoo(int width, int height) : FrameBuffer(width, height, false, false)
	{
        fb_cb = NULL;
        fb_buf = NULL;
	}

	FrameBufferMMiyoo::~FrameBufferMMiyoo()
	{
	}

	void *FrameBufferMMiyoo::lock()
	{
        if (fb_cb && fb_buf) {
            stride = 640 * 4;
            return framebuffer = fb_buf;
        }
        return NULL;
	}

	void FrameBufferMMiyoo::unlock()
	{
		framebuffer = nullptr;
	}

	void FrameBufferMMiyoo::blit(sw::Surface *source, const Rect *sourceRect, const Rect *destRect)
	{
        if (fb_cb && fb_buf) {
            copy(source);
            fb_buf = fb_cb();
        }
	}

    void FrameBufferMMiyoo::updateBufferSettings(void *p0, void *p1, void *p2)
    {
        fb_cb = (pFunc)p0;
        if (fb_cb) {
            fb_buf = fb_cb();
        }
        printf("[GPU] Updated Buffer Settings (cb %p, buf %p)\n", fb_cb, fb_buf);
    }
}

NO_SANITIZE_FUNCTION sw::FrameBuffer *createFrameBuffer(void *display, unsigned long window, int width, int height)
{
    if (window != 0) {
        return NULL;
    }
    printf("[GPU] Create Buffer (width %d, height %d)\n", width, height);
	return new sw::FrameBufferMMiyoo(width, height);
}

