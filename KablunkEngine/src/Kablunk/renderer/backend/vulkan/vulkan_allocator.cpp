#include "kablunkpch.h"

#include "kablunk/renderer/backend/vulkan/vulkan_allocator.h"

#include "kablunk/renderer/backend/vulkan/vulkan_context.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

struct vulkan_allocator_data
{
	VmaAllocator allocator;
	uint64_t total_allocated_bytes = 0;
};

static vulkan_allocator_data* s_data = nullptr;

vulkan_allocator::vulkan_allocator(const std::string& tag)
	: m_tag{ tag }
{

}

vulkan_allocator::~vulkan_allocator()
{

}

VmaAllocation vulkan_allocator::allocate_buffer(
    VkBufferCreateInfo p_buffer_create_info,
    const VmaMemoryUsage p_usage,
    VkBuffer& p_out_buffer
)
{
	VmaAllocationCreateInfo alloc_create_info{};
	alloc_create_info.usage = p_usage;
	alloc_create_info.pool = nullptr;
	alloc_create_info.memoryTypeBits = 0;

	VmaAllocation allocation;
    const VkResult res = vmaCreateBuffer(s_data->allocator, &p_buffer_create_info, &alloc_create_info, &p_out_buffer, &allocation, nullptr);
	if (res != VK_SUCCESS)
	{
		KB_CORE_ERROR("VulkanAllocator failed to create buffer!");
		KB_CORE_ERROR("  {0}", static_cast<u64>(res));
	}

	VmaAllocationInfo alloc_info{};
	vmaGetAllocationInfo(s_data->allocator, allocation, &alloc_info);
	KB_CORE_INFO("{0} allocted a buffer of size {1}", m_tag, alloc_info.size);

	s_data->total_allocated_bytes += alloc_info.size;

	return allocation;
}

VmaAllocation vulkan_allocator::allocate_image(const VkImageCreateInfo& image_create_info, VmaMemoryUsage usage, VkImage& out_image)
{
	VmaAllocationCreateInfo alloc_create_info{};
	alloc_create_info.usage = usage;

	VmaAllocation allocation;
	if (vmaCreateImage(s_data->allocator, &image_create_info, &alloc_create_info, &out_image, &allocation, nullptr) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "VulkanAllocator failed to create image!");

	VmaAllocationInfo alloc_info{};
	vmaGetAllocationInfo(s_data->allocator, allocation, &alloc_info);

	s_data->total_allocated_bytes += alloc_info.size;

	return allocation;
}

void vulkan_allocator::free(VmaAllocation allocation)
{
	vmaFreeMemory(s_data->allocator, allocation);
}

void vulkan_allocator::destroy_image(VkImage image, VmaAllocation allocation)
{
	KB_CORE_ASSERT(image, "trying to destroy image that is nullptr");
	KB_CORE_ASSERT(allocation, "trying to destroy allocation that is nullptr");
    KB_CORE_INFO("[VulkanImage2D]: destroying image {}", static_cast<void*>(image));
	vmaDestroyImage(s_data->allocator, image, allocation);
}

void vulkan_allocator::destroy_buffer(VkBuffer buffer, VmaAllocation allocation)
{
	KB_CORE_ASSERT(buffer, "trying to destroy image that is nullptr");
	KB_CORE_ASSERT(allocation, "trying to destroy allocation that is nullptr");
	vmaDestroyBuffer(s_data->allocator, buffer, allocation);
}

void vulkan_allocator::unmap_memory(VmaAllocation allocation)
{
	vmaUnmapMemory(s_data->allocator, allocation);
}

void vulkan_allocator::dump_stats()
{
	KB_CORE_ERROR("VulkanAllocation DumpStats not implemented!");
}

gpu_memory_stats_t vulkan_allocator::get_stats()
{
	KB_CORE_ERROR("VulkanAllocator GetStats not implemented!");
	return { 0, 0 };
}

void vulkan_allocator::init(arc<vulkan_logical_device> device)
{
	KB_CORE_INFO("Initializing VulkanAllocator!");
	s_data = new vulkan_allocator_data{};

	VmaAllocatorCreateInfo alloc_info{};
	alloc_info.vulkanApiVersion = VK_API_VERSION_1_2;
	alloc_info.physicalDevice = device->get_physical_device()->get_vk_physical_device();
	alloc_info.device = device->get_vk_device();
    alloc_info.instance = device->get_vk_instance();

	vmaCreateAllocator(&alloc_info, &s_data->allocator);
}

void vulkan_allocator::shutdown()
{
	vmaDestroyAllocator(s_data->allocator);

	delete s_data;
	s_data = nullptr;
}

VmaAllocator& vulkan_allocator::get_vma_allocator()
{
	return s_data->allocator;
}

} // end namespace kb::render::backend::vk
