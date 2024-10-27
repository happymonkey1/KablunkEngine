#include "kablunkpch.h"

#include "kablunk/renderer/backend/vulkan/vulkan_storage_buffer_set.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

vulkan_storage_buffer_set::vulkan_storage_buffer_set(u32 frames)
	: m_frames{ frames }
{

}

void vulkan_storage_buffer_set::create(u32 size, u32 binding)
{
	for (u32 frame = 0; frame < m_frames; ++frame)
	{
		const auto storage_buffer = storage_buffer::create(size, binding);
		set(storage_buffer, 0, frame); // #TODO only one set currently supported!
	}
}

arc<storage_buffer> vulkan_storage_buffer_set::get(u32 binding, u32 set, u32 frame)
{
	KB_CORE_ASSERT(m_storage_buffers.find(frame) != m_storage_buffers.end(), "frame not found!");
	KB_CORE_ASSERT(m_storage_buffers.at(frame).find(set) != m_storage_buffers.at(frame).end(), "set not found!");
	KB_CORE_ASSERT(m_storage_buffers.at(frame).at(set).find(binding) != m_storage_buffers.at(frame).at(set).end(), "binding not found");

	return m_storage_buffers[frame][set][binding];
}

void vulkan_storage_buffer_set::set(arc<storage_buffer> storage_buffer, u32 set /*= 0*/, u32 frame /*= 0*/)
{
	const u32 binding = storage_buffer->get_binding();

	m_storage_buffers[frame][set][binding] = storage_buffer;
}

void vulkan_storage_buffer_set::resize(u32 binding, u32 set, u32 new_size)
{
	for (u32 frame = 0; frame < m_frames; ++frame)
		m_storage_buffers[frame][set][binding]->resize(new_size);
}

} // end namespace kb::render::backend::vk
