#include "kablunkpch.h"

#include "kablunk/renderer/backend/vulkan/vulkan_storage_buffer.h"

#include "Kablunk/renderer/render_command.h"
#include "Kablunk/Core/Logger.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

vulkan_storage_buffer::vulkan_storage_buffer(size_t size, uint32_t binding)
	: m_size{ size }, m_binding{ binding }
{
    arc instance{ this };
	render::submit([instance]() mutable { instance->RT_Invalidate(); });
}

vulkan_storage_buffer::~vulkan_storage_buffer()
{
	Release();
}

void vulkan_storage_buffer::set_data(const void* data, size_t size, uint32_t offset /*= 0*/)
{
	memcpy(m_local_storage, data, size);
    arc instance{ this };
	render::submit([instance, size, offset]() mutable
		{
			instance->rt_set_data(instance->m_local_storage, size, offset);
		});
}

void vulkan_storage_buffer::rt_set_data(const void* data, size_t size, uint32_t offset /*= 0*/)
{
	vulkan_allocator allocator("VulkanStorageBuffer");
	uint8_t* data_ptr = allocator.map_memory<uint8_t>(m_vk_memory_allocation);
	memcpy(data_ptr, (uint8_t*)data + offset, size);
	KB_CORE_INFO("VulkanStorageBuffer mapping gpu memory of size '{0}'", size);
	allocator.unmap_memory(m_vk_memory_allocation);

	delete[] m_local_storage;
}

void vulkan_storage_buffer::resize(size_t new_size)
{
	m_size = new_size;
    arc instance{ this };
	render::submit([instance]() mutable { instance->RT_Invalidate(); });
}

void vulkan_storage_buffer::Release()
{
	if (!m_vk_memory_allocation)
		return;

	render::submit_resource_free([vk_buffer = m_vk_buffer, vk_mem_alloc = m_vk_memory_allocation]()
		{
			vulkan_allocator alloc{ "StorageBuffer" };
			alloc.destroy_buffer(vk_buffer, vk_mem_alloc);
		});

	m_vk_buffer = nullptr;
	m_vk_memory_allocation = nullptr;
}

void vulkan_storage_buffer::RT_Invalidate()
{
	Release();

	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	buffer_info.size = m_size;

	vulkan_allocator allocator("StorageBuffer");
	m_vk_memory_allocation = allocator.allocate_buffer(buffer_info, VMA_MEMORY_USAGE_GPU_ONLY, m_vk_buffer);

	m_vk_descriptor_info.buffer = m_vk_buffer;
	m_vk_descriptor_info.offset = 0;
	m_vk_descriptor_info.range = m_size;
}

} // end namespace kb::render::backend::vk
