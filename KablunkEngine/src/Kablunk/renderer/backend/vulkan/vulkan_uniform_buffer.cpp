#include "kablunkpch.h"

#include "kablunk/renderer/backend/vulkan/vulkan_context.h"
#include "kablunk/renderer/backend/vulkan/vulkan_uniform_buffer.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

vulkan_uniform_buffer::vulkan_uniform_buffer(uint32_t p_size)
	: m_size{ p_size }
{
	m_local_storage = new uint8_t[p_size];

#if 0
    arc instance{ this };
	render::submit([instance]() mutable
		{
			instance->rt_invalidate();
		});
#endif
    rt_invalidate();
}

vulkan_uniform_buffer::~vulkan_uniform_buffer()
{
	release();
}

void vulkan_uniform_buffer::set_data(const void* p_data, uint32_t p_size, uint32_t p_offset /*= 0*/)
{
	memcpy(m_local_storage, p_data, p_size);

    arc instance{ this };
	render::submit([instance, p_size, p_offset]() mutable
		{
			instance->rt_set_data(instance->m_local_storage, p_size, p_offset);
		});
}

void vulkan_uniform_buffer::rt_set_data(const void* p_data, uint32_t p_size, uint32_t p_offset /*= 0*/)
{
	vulkan_allocator allocator{ "UniformBuffer" };
	uint8_t* data_ptr = allocator.map_memory<uint8_t>(m_vk_allocation);
    // can this be memmove?
	memcpy(data_ptr, static_cast<const uint8_t*>(p_data) + p_offset, p_size);
	allocator.unmap_memory(m_vk_allocation);
}

void vulkan_uniform_buffer::rt_invalidate()
{
	release();

	VkBufferCreateInfo buffer_create_info{};
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buffer_create_info.size = m_size;

	vulkan_allocator allocator{ "UniformBuffer" };
	m_vk_allocation = allocator.allocate_buffer(buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, m_buffer);

	m_descriptor_info.buffer = m_buffer;
	m_descriptor_info.offset = 0;
	m_descriptor_info.range = m_size;
}

void vulkan_uniform_buffer::release()
{
	if (!m_vk_allocation)
		return;

	render::submit_resource_free([buffer = m_buffer, mem_alloc = m_vk_allocation]()
		{
			vulkan_allocator allocator{ "UniformBuffer" };
			allocator.destroy_buffer(buffer, mem_alloc);
		});

	m_buffer = nullptr;
	m_vk_allocation = nullptr;

	delete[] m_local_storage;
	m_local_storage = nullptr;
}

} // end namespace kb::render::backend::vk
