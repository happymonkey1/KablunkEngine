#pragma once
#ifndef KABLUNK_RENDERER_RENDER_COMMAND_QUEUE_H
#define KABLUNK_RENDERER_RENDER_COMMAND_QUEUE_H

#include <stdint.h>

namespace Kablunk
{
	class RenderCommandQueue
	{
	public:
		using RenderCommandFn = void(*)(void*);

		RenderCommandQueue();
		~RenderCommandQueue();

		void* Allocate(RenderCommandFn func, uint32_t size);

		void Execute();
	private:
		uint8_t* m_command_buffer;
		uint8_t* m_command_buffer_ptr;
		uint32_t m_command_count;
	};
}

#endif
