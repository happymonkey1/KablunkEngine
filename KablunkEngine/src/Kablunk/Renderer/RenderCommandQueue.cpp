#include "kablunkpch.h"

#include "Kablunk/Renderer/RenderCommandQueue.h"

namespace Kablunk
{

	RenderCommandQueue::RenderCommandQueue()
		: m_command_buffer{ new uint8_t[10 * 1024 * 1024]{ 0 } }, m_command_buffer_ptr{ m_command_buffer }, m_command_count{ 0 }
	{

	}

	RenderCommandQueue::~RenderCommandQueue()
	{
		delete[] m_command_buffer;
	}

	void* RenderCommandQueue::Allocate(RenderCommandFn func, uint32_t size)
	{
		*(RenderCommandFn*)m_command_buffer_ptr = func;
		m_command_buffer_ptr += sizeof(RenderCommandFn);

		*(uint32_t*)m_command_buffer_ptr = size;
		m_command_buffer_ptr += sizeof(uint32_t);

		void* memory = m_command_buffer_ptr;
		m_command_buffer_ptr += size;

		m_command_count++;
		return memory;
	}

	void RenderCommandQueue::Execute()
	{
		uint8_t* buffer = m_command_buffer;

		for (uint32_t i = 0; i < m_command_count; ++i)
		{
			RenderCommandFn func = *(RenderCommandFn*)buffer;
			buffer += sizeof(RenderCommandFn);

			uint32_t size = *(uint32_t*)buffer;
			buffer += sizeof(uint32_t);

			func(buffer);
			buffer += size;
		}

		m_command_buffer_ptr = m_command_buffer;
		m_command_count = 0;
	}

}
