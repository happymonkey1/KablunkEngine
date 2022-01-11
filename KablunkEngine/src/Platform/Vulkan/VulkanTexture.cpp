#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanImage.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/VulkanAllocator.h"
#include "Platform/Vulkan/VulkanRenderer.h"

#include "Kablunk/Renderer/RenderCommand.h"

#include <stb_image.h>

namespace Kablunk
{

	VulkanTexture2D::VulkanTexture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data)
		: m_width{ width }, m_height{ height }, m_format{ format }
	{
		ImageSpecification spec;
		spec.format = m_format;
		spec.width = m_width;
		spec.height = m_height;
		spec.mips = 1; // #TODO mipmaps
		spec.debug_name = "FIXME";
		//if (properties.Storage)
		//	spec.usage = ImageUsage::Storage;
		m_image = Image2D::Create(spec);

		IntrusiveRef<VulkanTexture2D> instance = this;
		RenderCommand::Submit([instance]() mutable
			{
				instance->Invalidate();
			});
	}

	VulkanTexture2D::VulkanTexture2D(const std::string& path)
		: m_filepath{ path }
	{
		bool loaded = LoadImage(m_filepath);

		if (!loaded)
		{
			LoadImage("resources/texture/missing_texture.png");
		}

		ImageSpecification spec{};
		spec.format = m_format;
		spec.width = m_width;
		spec.height = m_height;
		spec.mips = 1; // #TODO mipmaps
		spec.debug_name = "FIXME";
		m_image = Image2D::Create(spec);


		IntrusiveRef<VulkanTexture2D> instance = this;
		RenderCommand::Submit([instance]() mutable
			{
				instance->Invalidate();
			});
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
		if (m_image)
			m_image->Release();

		m_image_data.Release();
	}

	void VulkanTexture2D::Resize(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;

		IntrusiveRef<VulkanTexture2D> instance = this;
		RenderCommand::Submit([instance]() mutable
			{
				instance->Invalidate();
			});
	}

	Buffer VulkanTexture2D::GetWriteableBuffer()
	{
		return m_image_data;
	}

	void VulkanTexture2D::SetData(void* data, uint32_t size)
	{

	}

	void VulkanTexture2D::Bind(uint32_t slot) const
	{

	}

	bool VulkanTexture2D::operator==(const Texture& other) const
	{
		KB_CORE_ASSERT(false, "not implemented");
		return false;
	}

	void VulkanTexture2D::Invalidate()
	{
		auto device = VulkanContext::Get()->GetDevice();
		auto vk_device = device->GetVkDevice();

		m_image->Release();

		uint32_t mip_count = 1; // #TODO mipmap levels

		ImageSpecification& image_spec = m_image->GetSpecification();
		image_spec.format = m_format;
		image_spec.width = m_width;
		image_spec.height = m_height;
		image_spec.mips = mip_count;
		if (!m_image_data)
			image_spec.usage = ImageUsage::Storage;

		IntrusiveRef<VulkanImage2D> image = m_image.As<VulkanImage2D>();
		image->RT_Invalidate();

		auto& info = image->GetImageInfo();

		/*VkImageCreateInfo image_create_info{};
		image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.extent.width = static_cast<uint32_t>(m_width);
		image_create_info.extent.height = static_cast<uint32_t>(m_height);
		image_create_info.extent.depth = 1;
		image_create_info.mipLevels = 1;
		image_create_info.arrayLayers = 1;
		image_create_info.format = Utils::VulkanImageFormat(m_format);
		image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL; // #TODO extend api so this is an option
		image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.flags = 0;

		m_vk_image = {};
		if (vkCreateImage(vk_device, &image_create_info, nullptr, &m_vk_image) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create image!");*/

		if (m_image_data)
		{
			VkDeviceSize size = static_cast<uint64_t>(m_size);

			VkMemoryAllocateInfo mem_alloc_info{};
			mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

			VulkanAllocator allocator{ "Texture2D" };

			// Create staging buffer
			VkBufferCreateInfo buffer_create_info{};
			buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_create_info.size = size;
			buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VkBuffer staging_buffer;
			VmaAllocation staging_buffer_allocation = allocator.AllocateBuffer(buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, staging_buffer);

			// Copy data to staging buffer
			uint8_t* dest_ptr = allocator.MapMemory<uint8_t>(staging_buffer_allocation);
			KB_CORE_ASSERT(m_image_data.get(), "image data is nullptr!");
			memcpy(dest_ptr, m_image_data.get(), size);
			allocator.UnmapMemory(staging_buffer_allocation);

			VkCommandBuffer copy_cmd = device->GetCommandBuffer(true);

			// Image memory barriers for the texture image

			// The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
			VkImageSubresourceRange subresource_range{};
			// Image only contains color data
			subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			// Start at first mip level
			subresource_range.baseMipLevel = 0;
			subresource_range.levelCount = 1;
			subresource_range.layerCount = 1;

			// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
			VkImageMemoryBarrier image_memory_barrier{};
			image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			image_memory_barrier.image = info.image;
			image_memory_barrier.subresourceRange = subresource_range;
			image_memory_barrier.srcAccessMask = 0;
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

			// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition 
			// Source pipeline stage is host write/read exection (VK_PIPELINE_STAGE_HOST_BIT)
			// Destination pipeline stage is copy command exection (VK_PIPELINE_STAGE_TRANSFER_BIT)
			vkCmdPipelineBarrier(
				copy_cmd,
				VK_PIPELINE_STAGE_HOST_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &image_memory_barrier);

			VkBufferImageCopy buffer_copy_region_info = {};
			buffer_copy_region_info.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			buffer_copy_region_info.imageSubresource.mipLevel = 0;
			buffer_copy_region_info.imageSubresource.baseArrayLayer = 0;
			buffer_copy_region_info.imageSubresource.layerCount = 1;
			buffer_copy_region_info.imageExtent.width = m_width;
			buffer_copy_region_info.imageExtent.height = m_height;
			buffer_copy_region_info.imageExtent.depth = 1;
			buffer_copy_region_info.bufferOffset = 0;

			// Copy mip levels from staging buffer
			vkCmdCopyBufferToImage(copy_cmd, staging_buffer, info.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy_region_info);

			// #TODO mipmap levels, final image layout
			Utils::InsertImageMemoryBarrier(copy_cmd, info.image,
				VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				subresource_range);

			device->FlushCommandBuffer(copy_cmd);

			allocator.DestroyBuffer(staging_buffer, staging_buffer_allocation);
		}
		else
		{
			// #TODO mipmap levels, final image layout
			VkCommandBuffer transition_cmd_buffer = device->GetCommandBuffer(true);
			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.layerCount = 1;
			subresourceRange.levelCount = 1; // #TODO mipmap levels
			Utils::SetImageLayout(transition_cmd_buffer, info.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
			device->FlushCommandBuffer(transition_cmd_buffer);
		}

		// Release local storage
		m_image_data.Release();
	}

	bool VulkanTexture2D::LoadImage(const std::string& filepath)
	{
		int width, height, channels;
		if (stbi_is_hdr(filepath.c_str()))
		{
			void* data = stbi_loadf(filepath.c_str(), &width, &height, &channels, 4);

			m_size = width * height * 4 * sizeof(float);
			m_image_data.Allocate(m_size);

			m_image_data.Write(data, m_size, 0);
			m_format = ImageFormat::RGBA32F;
		}
		else
		{
			void* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);

			m_size = width * height * 4;
			m_image_data.Allocate(m_size);

			m_image_data.Write(data, m_size, 0);
			m_format = ImageFormat::RGBA;
		}

		if (!m_image_data.get())
			return false;

		m_width = width;
		m_height = height;
		return true;
	}

}
