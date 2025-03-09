// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#include "FrameBufferMini.hpp"
#include "Common/Timer.hpp"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

namespace sw {
	FrameBufferMini::FrameBufferMini(int width, int height) : FrameBuffer(width, height, false, false)
	{
        fb_cb = NULL;
        fb_buf = NULL;
	}

	FrameBufferMini::~FrameBufferMini()
	{
	}

	void *FrameBufferMini::lock()
	{
        if (fb_cb && fb_buf) {
            stride = 640 * 4;
            return framebuffer = fb_buf;
        }
        return NULL;
	}

	void FrameBufferMini::unlock()
	{
		framebuffer = nullptr;
	}

	void FrameBufferMini::blit(sw::Surface *source, const Rect *sourceRect, const Rect *destRect)
	{
        if (fb_cb && fb_buf) {
            copy(source);
            fb_buf = fb_cb();
        }
	}

    void FrameBufferMini::updateBufferSettings(void *p0, void *p1, void *p2)
    {
        fb_cb = (pFunc)p0;
        if (fb_cb) {
            fb_buf = fb_cb();
        }
        debug("%s, cb=%p, buf=%p\n", __func__, fb_cb, fb_buf);
    }
}

NO_SANITIZE_FUNCTION sw::FrameBuffer *createFrameBuffer(void *display, unsigned long window, int width, int height)
{
    if (window != 0) {
        return NULL;
    }

    debug("%s, w=%d, h=%d\n", __func__, width, height);
	return new sw::FrameBufferMini(width, height);
}

