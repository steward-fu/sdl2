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

#ifndef sw_FrameBufferMMiyoo_hpp
#define sw_FrameBufferMMiyoo_hpp

#include <stdint.h>
#include "Main/FrameBuffer.hpp"
#include "Common/Debug.hpp"

typedef void (*pFunc)(void);

namespace sw
{
	class FrameBufferMMiyoo : public FrameBuffer
	{
	public:
		FrameBufferMMiyoo(int width, int height);
		~FrameBufferMMiyoo() override;

        void updateBufferSettings(void *p0, void *p1, void *p2);
		void flip(sw::Surface *source) override { blit(source, nullptr, nullptr); }
		void blit(sw::Surface *source, const Rect *sourceRect, const Rect *destRect) override;

		void *lock() override;
		void unlock() override;

    private:
        pFunc fb_flip;
        int *fb_idx;
        void *fb_vaddr[2];
	};
}

#endif   // sw_FrameBufferMMiyoo_hpp
