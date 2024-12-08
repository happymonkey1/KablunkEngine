#include "kablunkpch.h"

#include "kablunk/renderer/backend/vulkan/vulkan_image.h"
#include "kablunk/renderer/backend/vulkan/vulkan_texture.h"
#include "kablunk/renderer/backend/vulkan/vulkan_allocator.h"
#include "Kablunk/renderer/backend/vulkan/vulkan_utils.h"

#include "Kablunk/renderer/render_command.h"

#include <stb_image.h>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

vulkan_texture_2d::vulkan_texture_2d(
    weak_ptr<vulkan_logical_device> p_device,
    image_format_t format,
    u32 width,
    u32 height,
    const void* data
)
	: m_hash{std::hash<std::string>{}(fmt::format("{}", data))}, m_width{width}, m_height{height},
      m_device{p_device},
      m_format{format}
{
	const size_t size = backend::util::GetImageMemorySize(format, width, height);

	if (data)
		m_image_data = owning_buffer::copy(data, size);

	image_specification_t spec;
	spec.format = m_format;
	spec.width = m_width;
	spec.height = m_height;
	spec.mips = 1; // #TODO mipmaps
	spec.debug_name = "FIXME";
	//if (properties.Storage)
	//	spec.usage = ImageUsage::Storage;
	m_image = image_2d::create(spec).As<vulkan_image_2d>();

    arc instance{ this };
	render::submit([instance]() mutable
		{
			instance->invalidate();
		});

	m_loaded = true;
}

vulkan_texture_2d::vulkan_texture_2d(weak_ptr<vulkan_logical_device> p_device, std::string path)
	: m_filepath{ std::move(path) }, m_hash{ static_cast<uint64_t>(std::hash<std::string>{}(m_filepath)) },
    m_device{ p_device }
{
    KB_CORE_INFO("[VulkanTexture2D]: Creating texture for '{}'", path);

	m_loaded = load_image(m_filepath);

	if (!m_loaded)
		m_loaded = load_image("resources/texture/missing_texture.png");

	image_specification_t spec{};
	spec.format = m_format;
	spec.width = m_width;
	spec.height = m_height;
	spec.mips = 1; // #TODO mipmaps
	spec.debug_name = "UNKNOWN_DEBUG_IMG_NAME";
	m_image = image_2d::create(spec).As<vulkan_image_2d>();


    arc instance{ this };
	render::submit([instance]() mutable
		{
			instance->invalidate();
		});
}

vulkan_texture_2d::~vulkan_texture_2d()
{
	if (m_image)
		m_image->release();

	m_image_data.release();
}

void vulkan_texture_2d::resize(u32 width, u32 height)
{
	m_width = width;
	m_height = height;

    arc instance{ this };
	render::submit([instance]() mutable
		{
			instance->invalidate();
		});
}

owning_buffer& vulkan_texture_2d::get_writeable_buffer()
{
	return m_image_data;
}

void vulkan_texture_2d::set_data(void* data, u32 size)
{
    m_image_data = owning_buffer::copy(data, size);

    arc instance{ this };
    render::submit([instance]() mutable
        {
            instance->invalidate();
        });
}

void vulkan_texture_2d::bind(u32 slot) const
{
}

bool vulkan_texture_2d::operator==(const texture_2d& other) const
{
	return m_image == other.get_image();
}

void vulkan_texture_2d::invalidate()
{
	auto vk_device = m_device->get_vk_device();

	m_image->release();

    u32 mip_count = 1; // #TODO mipmap levels

	image_specification_t& image_spec = m_image->get_specification();
	image_spec.format = m_format;
	image_spec.width = m_width;
	image_spec.height = m_height;
	image_spec.mips = mip_count;
	if (!m_image_data)
		image_spec.usage = image_usage_t::Storage;

	arc<vulkan_image_2d> image = m_image.As<vulkan_image_2d>();
	image->RT_Invalidate();

	auto& info = image->get_vk_image_info();

	if (m_image_data)
	{
		VkDeviceSize size = m_image_data.size();

		VkMemoryAllocateInfo mem_alloc_info{};
		mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

		vulkan_allocator allocator{ "Texture2D" };

		// Create staging buffer
		VkBufferCreateInfo buffer_create_info{};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = size;
		buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer staging_buffer;
		VmaAllocation staging_buffer_allocation = allocator.allocate_buffer(
            buffer_create_info,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            staging_buffer
        );

		// Copy data to staging buffer
		auto* dest_ptr = allocator.map_memory<u8>(staging_buffer_allocation);
		KB_CORE_ASSERT(m_image_data.get(), "image data is nullptr!");
		memcpy(dest_ptr, m_image_data.get(), size);
		KB_CORE_INFO("VulkanTexture2D mapping gpu memory of size '{0}'", size);
		allocator.unmap_memory(staging_buffer_allocation);

		VkCommandBuffer copy_cmd = m_device->get_vk_command_buffer(true);

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
		// Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
		// Destination pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
		vkCmdPipelineBarrier(
			copy_cmd,
			VK_PIPELINE_STAGE_HOST_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&image_memory_barrier
		);

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
		vkCmdCopyBufferToImage(
            copy_cmd,
            staging_buffer,
            info.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &buffer_copy_region_info
        );

		// #TODO mipmap levels, final image layout
        util::InsertImageMemoryBarrier(
			copy_cmd, 
			info.image,
			VK_ACCESS_TRANSFER_READ_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			image->get_vk_image_info_descriptor().imageLayout,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			subresource_range
		);

		m_device->flush_command_buffer(copy_cmd);

		allocator.destroy_buffer(staging_buffer, staging_buffer_allocation);
	}
	else
	{
		// #TODO mipmap levels, final image layout
		VkCommandBuffer transition_cmd_buffer = m_device->get_vk_command_buffer(true);
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.layerCount = 1;
		subresourceRange.levelCount = 1; // #TODO mipmap levels
        util::SetImageLayout(transition_cmd_buffer, info.image, VK_IMAGE_LAYOUT_UNDEFINED, image->get_vk_image_info_descriptor().imageLayout, subresourceRange);
        m_device->flush_command_buffer(transition_cmd_buffer);
	}

	// create texture sampler
	VkSamplerCreateInfo sampler{};
	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.maxAnisotropy = 1.0f;
	sampler.magFilter = VK_FILTER_NEAREST; // #TODO dynamic based on properties
	sampler.minFilter = VK_FILTER_NEAREST;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // #TODO dynamic based on properties
	sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.mipLodBias = 0.0f;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.maxAnisotropy = 1.0f;
	sampler.anisotropyEnable = VK_FALSE;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	if (vkCreateSampler(vk_device, &sampler, nullptr, &info.sampler) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "Vulkan failed to create sampler!");

	VkImageViewCreateInfo view_create_info{};
	view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_create_info.format = util::VulkanImageFormat(m_format);
	view_create_info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	// The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
	// It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
	view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view_create_info.subresourceRange.baseMipLevel = 0;
	view_create_info.subresourceRange.baseArrayLayer = 0;
	view_create_info.subresourceRange.layerCount = 1;
	view_create_info.subresourceRange.levelCount = 1; // #TODO dynamic
	view_create_info.image = info.image;
	if (vkCreateImageView(vk_device, &view_create_info, nullptr, &info.image_view) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "Vulkan failed to create image view!");

	image->UpdateDescriptor();

	KB_CORE_ASSERT(image->get_vk_image_info_descriptor().imageLayout != VK_IMAGE_LAYOUT_UNDEFINED, "layout still undefined!");

	// Release local storage
	m_image_data.release();
}

bool vulkan_texture_2d::load_image(const std::string& filepath)
{
	int width, height, channels;
	void* data;
	if (stbi_is_hdr(filepath.c_str()))
	{
		data = stbi_loadf(filepath.c_str(), &width, &height, &channels, 4);
		const auto size = static_cast<size_t>(width) * static_cast<size_t>(height) * 4ull * sizeof(float);
		m_image_data.allocate(size);

		m_image_data.write(data, size, 0);
		m_format = image_format_t::RGBA32F;
	}
	else
	{
        stbi_set_flip_vertically_on_load(1);
		data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
		const auto size = static_cast<size_t>(width) * static_cast<size_t>(height) * 4ull;
		m_image_data.allocate(size);

		m_image_data.write(data, size, 0);
		m_format = image_format_t::RGBA;
	}

    stbi_image_free(data);

	if (!m_image_data.get())
	{
		KB_CORE_ASSERT(false, "Image loaded but data is null!");
		return false;
	}

	m_width = width;
	m_height = height;
	return true;
}

} // end namespace kb::render::backend::vk
