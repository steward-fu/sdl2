// LGPL-2.1 License
// (C) 2025 Steward Fu <steward.fu@gmail.com>

#ifndef __SW_FRAMEBUFFER_MINI_HPP__
#define __SW_FRAMEBUFFER_MINI_HPP__

#include <stdint.h>
#include "Main/FrameBuffer.hpp"
#include "Common/Debug.hpp"

#if 0
    #define debug(...) printf(__VA_ARGS__)
#else
    #define debug(...) (void)0
#endif

typedef void* (*pFunc)(void);

namespace sw
{
	class FrameBufferMini : public FrameBuffer
	{
	public:
		FrameBufferMini(int width, int height);
		~FrameBufferMini() override;

        void updateBufferSettings(void *p0, void *p1, void *p2);
		void flip(sw::Surface *source) override { blit(source, nullptr, nullptr); }
		void blit(sw::Surface *source, const Rect *sourceRect, const Rect *destRect) override;

		void *lock() override;
		void unlock() override;

    private:
        pFunc fb_cb;
        void *fb_buf;
	};
}

#endif

