#include "kablunkpch.h"

#include "kablunk/renderer/backend/vulkan/vulkan_index_buffer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_context.h"
#include "kablunk/renderer/backend/vulkan/vulkan_allocator.h"

#include "Kablunk/renderer/render_command.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

// #TODO size aligned with device memory allocation chunks, see imgui for help
vulkan_index_buffer::vulkan_index_buffer(uint32_t size)
	: m_size{ size }
{

}

vulkan_index_buffer::vulkan_index_buffer(const void* data, uint32_t size /*= 0*/)
	: m_size{ size }
{
	m_local_data = owning_buffer::copy(data, size);
	KB_CORE_ASSERT(m_size == m_local_data.size(), "sizes do not match!");
    arc instance{ this };
	render::submit([instance]() mutable
		{
			auto device = vulkan_context::get()->get_device();
			vulkan_allocator allocator{ "IndexBuffer" };

			// create staging buffer
			VkBufferCreateInfo staging_buffer_create_info{};
			staging_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			staging_buffer_create_info.size = instance->m_size;
			staging_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			staging_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VkBuffer staging_buffer;
            const VmaAllocation staging_buffer_allocation = allocator.allocate_buffer(staging_buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, staging_buffer);

			// copy data to staging buffer (cpu)
			uint8_t* dest_ptr = allocator.map_memory<uint8_t>(staging_buffer_allocation);
			memcpy(dest_ptr, instance->m_local_data.get(), instance->m_local_data.size());
			KB_CORE_INFO("VulkanIndexBuffer mapping gpu memory of size '{0}'", instance->m_local_data.size());
			allocator.unmap_memory(staging_buffer_allocation);

			// Create vertex buffer info
			VkBufferCreateInfo index_buffer_create_info{};
			index_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			index_buffer_create_info.size = instance->m_size;
			index_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			instance->m_vk_allocation = allocator.allocate_buffer(index_buffer_create_info, VMA_MEMORY_USAGE_GPU_ONLY, instance->m_vk_buffer);

			// setup vk command to copy data from staging (cpu) to vertex buffer on gpu
			VkCommandBuffer copy_cmd = device->get_vk_command_buffer(true);

			VkBufferCopy copy_region{};
			copy_region.size = instance->m_local_data.size();
			vkCmdCopyBuffer(copy_cmd, staging_buffer, instance->m_vk_buffer, 1, &copy_region);

			device->flush_command_buffer(copy_cmd);

			allocator.destroy_buffer(staging_buffer, staging_buffer_allocation);
		});
}

vulkan_index_buffer::~vulkan_index_buffer()
{
    if (!m_vk_buffer)
        return;

	VkBuffer buffer = m_vk_buffer;
	VmaAllocation allocation = m_vk_allocation;
	render::submit_resource_free([buffer, allocation]() mutable
		{
			vulkan_allocator allocator{ "IndexBuffer" };
			allocator.destroy_buffer(buffer, allocation);
            buffer = nullptr;
		});

	m_local_data.release();
    m_vk_buffer = nullptr;
}

void vulkan_index_buffer::bind() const
{
    KB_CORE_WARN("VulkanIndexBuffer Bind not implemented!");
}

void vulkan_index_buffer::unbind() const
{
	KB_CORE_WARN("VulkanIndexBuffer Unbind not implemented!");
}

void vulkan_index_buffer::set_data(const void* buffer, uint32_t size, uint32_t offset /*= 0*/)
{
	KB_CORE_WARN("VulkanIndexBuffer SetData not implemented!");
}

auto vulkan_index_buffer::get_vk_index_type() const noexcept -> VkIndexType
{
    switch (m_index_type)
    {
    case index_type_t::u8:
        return VK_INDEX_TYPE_UINT8_EXT;
    case index_type_t::u16:
        return VK_INDEX_TYPE_UINT16;
    case index_type_t::u32:
        return VK_INDEX_TYPE_UINT32;
    default:
        KB_CORE_ASSERT(false, "[vulkan_index_buffer]: Unknown index buffer index type!");
        return VK_INDEX_TYPE_NONE_KHR;
    }
}

} // end namespace kb::render::backend::vk
