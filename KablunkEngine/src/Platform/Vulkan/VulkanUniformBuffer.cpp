#include "kablunkpch.h"


#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"


namespace Kablunk
{

	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size, uint32_t binding)
		: m_size{ size }, m_binding{ binding }
	{
		m_local_storage = new uint8_t[size];

		ref<VulkanUniformBuffer> instance = this;
		render::submit([instance]() mutable
			{
				instance->RT_Invalidate();
			});
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		Release();
	}

	void VulkanUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset /*= 0*/)
	{
		memcpy(m_local_storage, data, size);

		ref<VulkanUniformBuffer> instance = this;
		render::submit([instance, data, size, offset]() mutable
			{
				instance->RT_SetData(instance->m_local_storage, size, offset);
			});
	}

	void VulkanUniformBuffer::RT_SetData(const void* data, uint32_t size, uint32_t offset /*= 0*/)
	{
		VulkanAllocator allocator{ "UniformBuffer" };
		uint8_t* data_ptr = allocator.MapMemory<uint8_t>(m_vk_allocation);
		memcpy(data_ptr, (const uint8_t*)data + offset, size);
		allocator.UnmapMemory(m_vk_allocation);
	}

	void VulkanUniformBuffer::RT_Invalidate()
	{
		Release();

		VkBufferCreateInfo buffer_create_info{};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buffer_create_info.size = m_size;

		VulkanAllocator allocator{ "UniformBuffer" };
		m_vk_allocation = allocator.AllocateBuffer(buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, m_buffer);

		m_descriptor_info.buffer = m_buffer;
		m_descriptor_info.offset = 0;
		m_descriptor_info.range = m_size;
	}

	void VulkanUniformBuffer::Release()
	{
		if (!m_vk_allocation)
			return;

		render::submit_resource_free([buffer = m_buffer, mem_alloc = m_vk_allocation]()
			{
				VulkanAllocator allocator{ "UniformBuffer" };
				allocator.DestroyBuffer(buffer, mem_alloc);
			});

		m_buffer = nullptr;
		m_vk_allocation = nullptr;

		delete[] m_local_storage;
		m_local_storage = nullptr;
	}

}
