#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_ALLOCATOR_H
#define KABLUNK_PLATFORM_VULKAN_ALLOCATOR_H

#include <vulkan/vulkan.h>
#include <Platform/Vulkan/VulkanDevice.h>
#include <vendor/VulkanMemoryAllocator/vk_mem_alloc.h>

#include <string>

namespace kb
{
	struct GPUMemoryStats
	{
		uint64_t used = 0;
		uint64_t free = 0;

		uint64_t GetCurrentUsageBytes() { return used - free; }
	};

	class VulkanAllocator
	{
	public:
		VulkanAllocator() = default;
		VulkanAllocator(const std::string& tag);
		~VulkanAllocator();

		VmaAllocation AllocateBuffer(VkBufferCreateInfo buffer_create_info, VmaMemoryUsage usage, VkBuffer& out_buffer);
		VmaAllocation AllocateImage(const VkImageCreateInfo& image_create_info, VmaMemoryUsage usage, VkImage& out_image);

		void Free(VmaAllocation allocation);
		void DestroyImage(VkImage image, VmaAllocation allocation);
		void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);

		template <typename T>
		T* MapMemory(VmaAllocation allocation)
		{
			T* mapped_mem;
			vmaMapMemory(VulkanAllocator::GetVMAAllocator(), allocation, (void**)&mapped_mem);
			return mapped_mem;
		}

		void UnmapMemory(VmaAllocation allocation);

		static void DumpStats();
		static GPUMemoryStats GetStats();

		static void Init(ref<VulkanDevice> device);
		static void Shutdown();

		static VmaAllocator& GetVMAAllocator();
	private:
		std::string m_tag;

	};
}

#endif
