#include "kablunkpch.h"

#include "Kablunk/Renderer/render_command_queue.h"

namespace kb
{
render_command_queue::render_command_queue()
	: m_command_buffer{ nullptr }, m_command_buffer_ptr{ nullptr }, m_command_count{ 0 }
{
	constexpr size_t k_ten_mb = 10ull * 1024ull * 1024ull;
	m_command_buffer = new uint8_t[k_ten_mb];
	m_command_buffer_ptr = m_command_buffer;
	memset(m_command_buffer, 0, k_ten_mb);

    KB_CORE_INFO("[render_command_queue]: initialized command buffer of {} bytes", k_ten_mb);
}

render_command_queue::render_command_queue(render_command_queue&& p_other) noexcept
    : m_command_buffer{ p_other.m_command_buffer }, m_command_buffer_ptr{ p_other.m_command_buffer_ptr }, m_command_count{ p_other.m_command_count }
{
    p_other.m_command_buffer = nullptr;
    p_other.m_command_buffer_ptr = nullptr;
    p_other.m_command_count = 0;
}

render_command_queue::~render_command_queue()
{
    delete[] m_command_buffer;
}

render_command_queue& render_command_queue::operator=(render_command_queue&& p_other) noexcept
{
    std::swap(m_command_buffer, p_other.m_command_buffer);
    std::swap(m_command_buffer_ptr, p_other.m_command_buffer_ptr);
    std::swap(m_command_count, p_other.m_command_count);

    return *this;
}

void* render_command_queue::allocate(RenderCommandFn func, uint32_t size)
{
	*reinterpret_cast<RenderCommandFn*>(m_command_buffer_ptr) = func;
	m_command_buffer_ptr += sizeof(RenderCommandFn);

	*reinterpret_cast<uint32_t*>(m_command_buffer_ptr) = size;
	m_command_buffer_ptr += sizeof(uint32_t);

	void* memory = m_command_buffer_ptr;
	m_command_buffer_ptr += size;

	m_command_count++;
	return memory;
}

void render_command_queue::execute()
{
    KB_PROFILE_FUNC()

	uint8_t* buffer = m_command_buffer;

	for (uint32_t i = 0; i < m_command_count; ++i)
	{
        const RenderCommandFn func = *reinterpret_cast<RenderCommandFn*>(buffer);
		buffer += sizeof(RenderCommandFn);

        const uint32_t size = *reinterpret_cast<uint32_t*>(buffer);
		buffer += sizeof(uint32_t);

		func(buffer);
		buffer += size;
	}

	m_command_buffer_ptr = m_command_buffer;
	m_command_count = 0;
}
}
