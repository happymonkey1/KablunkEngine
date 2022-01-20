#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanAllocator.h"

#include "Platform/Vulkan/VulkanContext.h"

namespace Kablunk
{
	struct VulkanAllocatorData
	{
		VmaAllocator allocator;
		uint64_t total_allocated_bytes = 0;
	};

	static VulkanAllocatorData* s_data = nullptr;

	VulkanAllocator::VulkanAllocator(const std::string& tag)
		: m_tag{ tag }
	{

	}

	VulkanAllocator::~VulkanAllocator()
	{

	}

	VmaAllocation VulkanAllocator::AllocateBuffer(VkBufferCreateInfo buffer_create_info, VmaMemoryUsage usage, VkBuffer& out_buffer)
	{
		VmaAllocationCreateInfo alloc_create_info{};
		alloc_create_info.usage = usage;
		alloc_create_info.pool = nullptr;
		alloc_create_info.memoryTypeBits = 0;

		VmaAllocation allocation;
		VkResult res = vmaCreateBuffer(s_data->allocator, &buffer_create_info, &alloc_create_info, &out_buffer, &allocation, nullptr);
		if (res != VK_SUCCESS)
		{
			KB_CORE_ERROR("VulkanAllocator failed to create buffer!");
			KB_CORE_ERROR("  {0}", res);
		}

		VmaAllocationInfo alloc_info{};
		vmaGetAllocationInfo(s_data->allocator, allocation, &alloc_info);
		KB_CORE_INFO("{0} allocted a buffer of size {1}", m_tag, alloc_info.size);

		s_data->total_allocated_bytes += alloc_info.size;

		return allocation;
	}

	VmaAllocation VulkanAllocator::AllocateImage(const VkImageCreateInfo& image_create_info, VmaMemoryUsage usage, VkImage& out_image)
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

	void VulkanAllocator::Free(VmaAllocation allocation)
	{
		vmaFreeMemory(s_data->allocator, allocation);
	}

	void VulkanAllocator::DestroyImage(VkImage image, VmaAllocation allocation)
	{
		KB_CORE_ASSERT(image, "trying to destroy image that is nullptr");
		KB_CORE_ASSERT(allocation, "trying to destroy allocation that is nullptr");
		vmaDestroyImage(s_data->allocator, image, allocation);
	}

	void VulkanAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
	{
		KB_CORE_ASSERT(buffer, "trying to destroy image that is nullptr");
		KB_CORE_ASSERT(allocation, "trying to destroy allocation that is nullptr");
		vmaDestroyBuffer(s_data->allocator, buffer, allocation);
	}

	void VulkanAllocator::UnmapMemory(VmaAllocation allocation)
	{
		vmaUnmapMemory(s_data->allocator, allocation);
	}

	void VulkanAllocator::DumpStats()
	{
		KB_CORE_ERROR("VulkanAllocation DumpStats not implemented!");
	}

	GPUMemoryStats VulkanAllocator::GetStats()
	{
		KB_CORE_ERROR("VulkanAllocator GetStats not implemented!");
		return { 0, 0 };
	}

	void VulkanAllocator::Init(IntrusiveRef<VulkanDevice> device)
	{
		KB_CORE_INFO("Initializing VulkanAllocator!");
		s_data = new VulkanAllocatorData{};

		VmaAllocatorCreateInfo alloc_info{};
		alloc_info.vulkanApiVersion = VK_API_VERSION_1_2;
		alloc_info.physicalDevice = device->GetPhysicalDevice()->GetVkDevice();
		alloc_info.device = device->GetVkDevice();
		alloc_info.instance = VulkanContext::GetInstance();

		vmaCreateAllocator(&alloc_info, &s_data->allocator);
	}

	void VulkanAllocator::Shutdown()
	{
		vmaDestroyAllocator(s_data->allocator);

		delete s_data;
		s_data = nullptr;
	}

	VmaAllocator& VulkanAllocator::GetVMAAllocator()
	{
		return s_data->allocator;
	}

}
