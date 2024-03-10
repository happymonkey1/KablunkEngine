#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanIndexBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanAllocator.h"

#include "Kablunk/Renderer/RenderCommand.h"

namespace kb
{
	
	// #TODO size aligned with device memory allocation chunks, see imgui for help

	VulkanIndexBuffer::VulkanIndexBuffer(uint32_t size)
		: m_size{ size }
	{

	}

	VulkanIndexBuffer::VulkanIndexBuffer(const void* data, uint32_t size /*= 0*/)
		: m_size{ size }
	{
		m_local_data = owning_buffer::Copy(data, size);
		KB_CORE_ASSERT(m_size == m_local_data.size(), "sizes do not match!");
        ref<VulkanIndexBuffer> instance{ this };
		render::submit([instance]() mutable
			{
				auto device = VulkanContext::Get()->GetDevice();
				VulkanAllocator allocator{ "IndexBuffer" };

				// create staging buffer
				VkBufferCreateInfo staging_buffer_create_info{};
				staging_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				staging_buffer_create_info.size = instance->m_size;
				staging_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				staging_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				VkBuffer staging_buffer;
				VmaAllocation staging_buffer_allocation = allocator.AllocateBuffer(staging_buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, staging_buffer);

				// copy data to staging buffer (cpu)
				uint8_t* dest_ptr = allocator.MapMemory<uint8_t>(staging_buffer_allocation);
				memcpy(dest_ptr, instance->m_local_data.get(), instance->m_local_data.size());
				KB_CORE_INFO("VulkanIndexBuffer mapping gpu memory of size '{0}'", instance->m_local_data.size());
				allocator.UnmapMemory(staging_buffer_allocation);

				// Create vertex buffer info
				VkBufferCreateInfo index_buffer_create_info{};
				index_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				index_buffer_create_info.size = instance->m_size;
				index_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
				instance->m_vk_allocation = allocator.AllocateBuffer(index_buffer_create_info, VMA_MEMORY_USAGE_GPU_ONLY, instance->m_vk_buffer);

				// setup vk command to copy data from staging (cpu) to vertex buffer on gpu
				VkCommandBuffer copy_cmd = device->GetCommandBuffer(true);

				VkBufferCopy copy_region{};
				copy_region.size = instance->m_local_data.size();
				vkCmdCopyBuffer(copy_cmd, staging_buffer, instance->m_vk_buffer, 1, &copy_region);

				device->FlushCommandBuffer(copy_cmd);

				allocator.DestroyBuffer(staging_buffer, staging_buffer_allocation);
			});
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		VkBuffer buffer = m_vk_buffer;
		VmaAllocation allocation = m_vk_allocation;
		render::submit([buffer, allocation]() mutable
			{
				VulkanAllocator allocator{ "IndexBuffer" };
				allocator.DestroyBuffer(buffer, allocation);
			});

		m_local_data.Release();
	}

	void VulkanIndexBuffer::Bind() const
	{
		KB_CORE_WARN("VulkanIndexBuffer Bind not implemented!");
	}

	void VulkanIndexBuffer::Unbind() const
	{
		KB_CORE_WARN("VulkanIndexBuffer Unbind not implemented!");
	}

	void VulkanIndexBuffer::SetData(const void* buffer, uint32_t size, uint32_t offset /*= 0*/)
	{
		KB_CORE_WARN("VulkanIndexBuffer SetData not implemented!");
	}

	RendererID VulkanIndexBuffer::GetRendererID() const
	{
		KB_CORE_WARN("VulkanIndexBuffer GetRendererID not implemented!");
		return 0;
	}

}
