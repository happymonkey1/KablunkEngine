#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_ALLOCATOR_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_ALLOCATOR_H

#include "kablunk/renderer/backend/vulkan/vulkan_logical_device.h"

#include <vendor/VulkanMemoryAllocator/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <string>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

struct GPUMemoryStats
{
	uint64_t used = 0;
	uint64_t free = 0;

	uint64_t GetCurrentUsageBytes() { return used - free; }
};

class vulkan_allocator
{
public:
	vulkan_allocator() = default;
	vulkan_allocator(const std::string& tag);
	~vulkan_allocator();

	VmaAllocation allocate_buffer(VkBufferCreateInfo buffer_create_info, VmaMemoryUsage usage, VkBuffer& out_buffer);
	VmaAllocation allocate_image(const VkImageCreateInfo& image_create_info, VmaMemoryUsage usage, VkImage& out_image);

	void free(VmaAllocation allocation);
	void destroy_image(VkImage image, VmaAllocation allocation);
	void destroy_buffer(VkBuffer buffer, VmaAllocation allocation);

	template <typename T>
	T* map_memory(VmaAllocation allocation)
	{
		T* mapped_mem;
		vmaMapMemory(vulkan_allocator::get_vma_allocator(), allocation, (void**)&mapped_mem);
		return mapped_mem;
	}

	void unmap_memory(VmaAllocation allocation);

	static void dump_stats();
	static GPUMemoryStats get_stats();

	static void init(arc<vulkan_logical_device> device);
	static void shutdown();

	static VmaAllocator& get_vma_allocator();

private:
	std::string m_tag;
};

} // end namespace kb::render::backend::vk

#endif
