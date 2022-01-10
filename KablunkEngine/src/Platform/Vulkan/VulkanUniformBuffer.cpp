#include "kablunkpch.h"


#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"


namespace Kablunk
{

	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size, uint32_t binding)
		: m_size{ size }, m_binding{ binding }
	{
		m_local_storage = new uint8_t[size];

		IntrusiveRef<VulkanUniformBuffer> instance = this;
		RenderCommand::Submit([instance]() mutable
			{
				instance->Invalidate();
			});
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		Release();
	}

	void VulkanUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offest /*= 0*/)
	{
		IntrusiveRef<VulkanUniformBuffer> instance = this;
		RenderCommand::Submit([instance, data, size, offest]() mutable
			{
				VkDevice device = VulkanContext::Get()->GetDevice()->GetVkDevice();

				VkBufferCreateInfo buffer_create_info{};
				buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
				buffer_create_info.size = instance->m_size;
				
				// create buffer
				vkCreateBuffer(device, &buffer_create_info, nullptr, &instance->m_buffer);

				// allocate memory
				VkMemoryAllocateInfo alloc_info{};
				alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				alloc_info.pNext = nullptr;
				alloc_info.allocationSize = instance->m_size;
				alloc_info.memoryTypeIndex = 0;

				// #TODO replace with VulkanAllocator
				if (vkAllocateMemory(device, &alloc_info, nullptr, &instance->m_vk_memory) != VK_SUCCESS)
					KB_CORE_ERROR("Vulkan uniform buffer failed to allocate memory!");
				
				vkBindBufferMemory(device, instance->m_buffer, instance->m_vk_memory, offest);

				vkMapMemory(device, instance->m_vk_memory, offest, size, 0, (void**)&instance->m_local_storage);

				memcpy(instance->m_local_storage, data, instance->m_size);

				vkUnmapMemory(device, instance->m_vk_memory);

				// flush mapped memory?
			});
	}

	void VulkanUniformBuffer::Invalidate()
	{
		Release();

		VkDevice device = VulkanContext::Get()->GetDevice()->GetVkDevice();

		VkMemoryAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.pNext = nullptr;
		alloc_info.allocationSize = 0;
		alloc_info.memoryTypeIndex = 0;

		VkBufferCreateInfo buffer_create_info{};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buffer_create_info.size = m_size;

		if (m_buffer)
		{
			vkDestroyBuffer(device, m_buffer, nullptr);
			vkFreeMemory(device, m_vk_memory, nullptr);
		}

		m_descriptor_info.buffer = m_buffer;
		m_descriptor_info.offset = 0;
		m_descriptor_info.range = m_size;
	}

	void VulkanUniformBuffer::Release()
	{
		VkDevice device = VulkanContext::Get()->GetDevice()->GetVkDevice();
		vkDestroyBuffer(device, m_buffer, nullptr);

		m_buffer = nullptr;

		delete[] m_local_storage;
		m_local_storage = nullptr;
	}

}
