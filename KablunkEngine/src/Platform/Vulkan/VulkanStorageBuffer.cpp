#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanStorageBuffer.h"

#include "Kablunk/Renderer/RenderCommand.h"
#include "Kablunk/Core/Logger.h"

namespace Kablunk
{

	VulkanStorageBuffer::VulkanStorageBuffer(size_t size, uint32_t binding)
		: m_size{ size }, m_binding{ binding }
	{
		IntrusiveRef<VulkanStorageBuffer> instance = this;
		render::submit([instance]() mutable { instance->RT_Invalidate(); });
	}

	VulkanStorageBuffer::~VulkanStorageBuffer()
	{
		Release();
	}

	void VulkanStorageBuffer::SetData(const void* data, size_t size, uint32_t offset /*= 0*/)
	{
		memcpy(m_local_storage, data, size);
		IntrusiveRef<VulkanStorageBuffer> instance = this;
		render::submit([instance, size, offset]() mutable
			{
				instance->RT_SetData(instance->m_local_storage, size, offset);
			});

	}

	void VulkanStorageBuffer::RT_SetData(const void* data, size_t size, uint32_t offset /*= 0*/)
	{
		VulkanAllocator allocator("VulkanStorageBuffer");
		uint8_t* data_ptr = allocator.MapMemory<uint8_t>(m_vk_memory_allocation);
		memcpy(data_ptr, (uint8_t*)data + offset, size);
		KB_CORE_INFO("VulkanStorageBuffer mapping gpu memory of size '{0}'", size);
		allocator.UnmapMemory(m_vk_memory_allocation);

		delete[] m_local_storage;
	}

	void VulkanStorageBuffer::Resize(size_t new_size)
	{
		m_size = new_size;
		IntrusiveRef<VulkanStorageBuffer> instance = this;
		render::submit([instance]() mutable { instance->RT_Invalidate(); });
	}

	void VulkanStorageBuffer::Release()
	{
		if (!m_vk_memory_allocation)
			return;

		render::submit_resource_free([vk_buffer = m_vk_buffer, vk_mem_alloc = m_vk_memory_allocation]()
			{
				VulkanAllocator alloc{ "StorageBuffer" };
				alloc.DestroyBuffer(vk_buffer, vk_mem_alloc);
			});

		m_vk_buffer = nullptr;
		m_vk_memory_allocation = nullptr;
	}

	void VulkanStorageBuffer::RT_Invalidate()
	{
		Release();

		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		buffer_info.size = m_size;

		VulkanAllocator allocator("StorageBuffer");
		m_vk_memory_allocation = allocator.AllocateBuffer(buffer_info, VMA_MEMORY_USAGE_GPU_ONLY, m_vk_buffer);

		m_vk_descriptor_info.buffer = m_vk_buffer;
		m_vk_descriptor_info.offset = 0;
		m_vk_descriptor_info.range = m_size;
	}

}
