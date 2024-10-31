#include "kablunkpch.h"

#include "kablunk/renderer/backend/vulkan/vulkan_vertex_buffer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_context.h"
#include "Kablunk/renderer/render_command.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

vulkan_vertex_buffer::vulkan_vertex_buffer(
    const weak_arc<vulkan_logical_device> p_device,
    const void* data,
    const u32 size,
    vertex_buffer_usage_t usage /*= VertexBufferUsage::Static*/
)
	: m_size{ size }, m_device{ p_device }
{
	m_local_data = owning_buffer::copy(data, size);

	KB_CORE_ASSERT(m_local_data.size() == m_size, "sizes do not match!");

    arc instance{ this };
	render::submit([instance]() mutable
		{
			auto device = instance->m_device;
			vulkan_allocator allocator{ "VertexBuffer" };

			// create staging buffer
			VkBufferCreateInfo staging_buffer_create_info{};
			staging_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			staging_buffer_create_info.size = instance->m_size;
			staging_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			staging_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VkBuffer staging_buffer;
            const VmaAllocation staging_buffer_allocation = allocator.allocate_buffer(staging_buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, staging_buffer);

			// copy data to staging buffer (cpu)
			u8* dest_data = allocator.map_memory<uint8_t>(staging_buffer_allocation);
			memcpy(dest_data, instance->m_local_data.get(), instance->m_local_data.size());
			KB_CORE_INFO("VulkanVertexBuffer mapping staging memory of size '{}'", instance->m_local_data.size());
			allocator.unmap_memory(staging_buffer_allocation);

			// Create vertex buffer info
			VkBufferCreateInfo vertex_buffer_create_info{};
			vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			vertex_buffer_create_info.size = instance->m_size;
			vertex_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			instance->m_memory_allocation = allocator.allocate_buffer(vertex_buffer_create_info, VMA_MEMORY_USAGE_GPU_ONLY, instance->m_vk_buffer);

			// setup vk command to copy data from staging (cpu) to vertex buffer on gpu
            const VkCommandBuffer copy_cmd = device->get_vk_command_buffer(true);

			VkBufferCopy copy_region{};
			copy_region.size = instance->m_local_data.size();
			vkCmdCopyBuffer(
                copy_cmd,
                staging_buffer,
                instance->m_vk_buffer,
                1,
                &copy_region
            );

			KB_CORE_INFO("VertexBuffer about to flush command queue!");
			device->flush_command_buffer(copy_cmd);

			KB_CORE_INFO("VertexBuffer destroying staging buffer!");
			allocator.destroy_buffer(staging_buffer, staging_buffer_allocation);
		});
}

vulkan_vertex_buffer::vulkan_vertex_buffer(
    const weak_arc<vulkan_logical_device> p_device,
    const u32 size,
    vertex_buffer_usage_t usage /*= VertexBufferUsage::Dynamic*/
)
	: m_size{ size }, m_device{ p_device }, m_memory_allocation{ nullptr }
{
	m_local_data.allocate(size);

    arc instance{ this };
	render::submit([instance]() mutable
		{
			VkDevice device = instance->m_device->get_vk_device();
			vulkan_allocator allocator{ "VertexBuffer" };

			VkBufferCreateInfo vertex_buffer_create_info{};
			vertex_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			vertex_buffer_create_info.size = instance->m_size;
			vertex_buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

			instance->m_memory_allocation = allocator.allocate_buffer(vertex_buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, instance->m_vk_buffer);
		});
}

vulkan_vertex_buffer::~vulkan_vertex_buffer()
{
	VkBuffer buffer = m_vk_buffer;
	VmaAllocation allocation = m_memory_allocation;
	render::submit([buffer, allocation]() mutable
		{
			vulkan_allocator allocator{ "VertexBuffer" };
			allocator.destroy_buffer(buffer, allocation);
		});

	m_local_data.release();
}

void vulkan_vertex_buffer::bind() const
{
	KB_CORE_WARN("VulkanVertexBuffer does not bind!");
}

void vulkan_vertex_buffer::unbind() const
{
	KB_CORE_WARN("VulkanVertexBuffer does not unbind!");
}

void vulkan_vertex_buffer::set_data(
    const void* data,
    u32 size,
    u32 offset /*= 0*/
)
{
    KB_CORE_ASSERT(
        size <= m_local_data.size(),
        "[VulkanVertexBuffer]: SetData size out of local buffer bounds!"
    );
    memcpy(
        m_local_data.get(),
        static_cast<const u8*>(data) + offset,
        size
    );
    arc instance{ this };
	render::submit([instance, size, offset]() mutable {
			instance->rt_set_data(instance->m_local_data.get(), size, offset);
		});
}

void vulkan_vertex_buffer::rt_set_data(
    const void* data,
    u32 size,
    u32 offset /*= 0*/
)
{
	vulkan_allocator allocator{ "VertexBuffer" };
	auto* data_ptr = allocator.map_memory<u8>(m_memory_allocation);
	memcpy(data_ptr, static_cast<const u8*>(data) + offset, size);
	allocator.unmap_memory(m_memory_allocation);
}

void vulkan_vertex_buffer::set_layout(const buffer_layout& layout)
{
	KB_CORE_WARN("VulkanVertexBuffer does not implement layouts");
}

const buffer_layout& vulkan_vertex_buffer::get_layout() const
{
	KB_CORE_WARN("VulkanVertexBuffer does not implement layouts");
	KB_CORE_ASSERT(false, "[VulkanVertexBuffer]: does not implement layouts");
	return {};
}

RendererID vulkan_vertex_buffer::get_renderer_id() const
{
	KB_CORE_WARN("VulkanVertexBuffer does not implement rendererID");
	return 0;
}

} // end namespace kb::render::backend::vk
