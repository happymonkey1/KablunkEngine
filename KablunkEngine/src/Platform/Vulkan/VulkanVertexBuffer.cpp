#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanVertexBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"

#include "Kablunk/Renderer/RenderCommand.h"

namespace Kablunk
{

	VulkanVertexBuffer::VulkanVertexBuffer(const void* data, uint32_t size, VertexBufferUsage usage /*= VertexBufferUsage::Static*/)
		: m_size{ size }
	{
		m_local_data = Buffer::Copy(data, size);

		KB_CORE_ASSERT(m_local_data.size() == m_size, "sizes do not match!");

		ref<VulkanVertexBuffer> instance = this;
		render::submit([instance]() mutable
			{
				ref<VulkanDevice> device = VulkanContext::Get()->GetDevice();
				VulkanAllocator allocator{ "VertexBuffer" };
				
				// create staging buffer
				VkBufferCreateInfo staging_buffer_create_info{};
				staging_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				staging_buffer_create_info.size = instance->m_size;
				staging_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				staging_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				VkBuffer staging_buffer;
				VmaAllocation staging_buffer_allocation = allocator.AllocateBuffer(staging_buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, staging_buffer);

				// copy data to staging buffer (cpu)
				uint8_t* dest_data = allocator.MapMemory<uint8_t>(staging_buffer_allocation);
				memcpy(dest_data, instance->m_local_data.get(), instance->m_local_data.size());
				KB_CORE_INFO("VulkanVertexBuffer mapping staging memory of size '{0}'", instance->m_local_data.size());
				allocator.UnmapMemory(staging_buffer_allocation);

				
				// Create vertex buffer info
				VkBufferCreateInfo vertex_buffer_create_info{};
				vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				vertex_buffer_create_info.size = instance->m_size; 
				vertex_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				instance->m_memory_allocation = allocator.AllocateBuffer(vertex_buffer_create_info, VMA_MEMORY_USAGE_GPU_ONLY, instance->m_vk_buffer);

				// setup vk command to copy data from staging (cpu) to vertex buffer on gpu
				VkCommandBuffer copy_cmd = device->GetCommandBuffer(true);

				VkBufferCopy copy_region{};
				copy_region.size = instance->m_local_data.size();
				vkCmdCopyBuffer(copy_cmd, staging_buffer, instance->m_vk_buffer, 1, &copy_region);

				KB_CORE_INFO("VertexBuffer about to flush command queue!");
				device->FlushCommandBuffer(copy_cmd);

				KB_CORE_INFO("VertexBuffer destroying staging buffer!");
				allocator.DestroyBuffer(staging_buffer, staging_buffer_allocation);
			});
	}

	VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size, VertexBufferUsage usage /*= VertexBufferUsage::Dynamic*/)
		: m_size{ size }, m_memory_allocation{ nullptr }
	{
		m_local_data.Allocate(size);

		ref<VulkanVertexBuffer> instance = this;
		render::submit([instance]() mutable
			{
				VkDevice device = VulkanContext::Get()->GetDevice()->GetVkDevice();
				VulkanAllocator allocator{ "VertexBuffer" };

				VkBufferCreateInfo vertex_buffer_create_info{};
				vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				vertex_buffer_create_info.size = instance->m_size;
				vertex_buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

				instance->m_memory_allocation = allocator.AllocateBuffer(vertex_buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, instance->m_vk_buffer);
			});
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		VkBuffer buffer = m_vk_buffer;
		VmaAllocation allocation = m_memory_allocation;
		render::submit([buffer, allocation]() mutable
			{
				VulkanAllocator allocator{ "VertexBuffer" };
				allocator.DestroyBuffer(buffer, allocation);
			});

		m_local_data.Release();
	}

	void VulkanVertexBuffer::Bind() const
	{
		KB_CORE_WARN("VulkanVertexBuffer does not bind!");
	}

	void VulkanVertexBuffer::Unbind() const
	{
		KB_CORE_WARN("VulkanVertexBuffer does not unbind!");
	}

	void VulkanVertexBuffer::SetData(const void* data, uint32_t size, uint32_t offset /*= 0*/)
	{
		memcpy(m_local_data.get(), (uint8_t*)data + offset, size);
		ref<VulkanVertexBuffer> instance = this;
		render::submit([instance, size, offset]() mutable {
				instance->RT_SetData(instance->m_local_data.get(), size, offset);
			});
	}

	void VulkanVertexBuffer::RT_SetData(const void* data, uint32_t size, uint32_t offset /*= 0*/)
	{
		VulkanAllocator allocator{ "VertexBuffer" };
		uint8_t* data_ptr = allocator.MapMemory<uint8_t>(m_memory_allocation);
		memcpy(data_ptr, (uint8_t*)data + offset, size);
		allocator.UnmapMemory(m_memory_allocation);
	}

	void VulkanVertexBuffer::SetLayout(const BufferLayout& layout)
	{
		KB_CORE_WARN("VulkanVertexBuffer does not implement layouts");
	}

	const BufferLayout& VulkanVertexBuffer::GetLayout() const
	{
		KB_CORE_WARN("VulkanVertexBuffer does not implement layouts");
		KB_CORE_ASSERT(false, "");
		return {};
	}

	RendererID VulkanVertexBuffer::GetRendererID() const
	{
		KB_CORE_WARN("VulkanVertexBuffer does not implement rendererID");
		return 0;
	}

}
