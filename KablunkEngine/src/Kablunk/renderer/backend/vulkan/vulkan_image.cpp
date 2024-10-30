#include "kablunkpch.h"

#include "kablunk/renderer/backend/vulkan/vulkan_image.h"

#include "Kablunk/Renderer/RenderCommand.h"
#include "kablunk/renderer/backend/vulkan/VulkanRenderer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_api.h"
#include "kablunk/renderer/backend/vulkan/vulkan_core.h"
#include "kablunk/renderer/backend/vulkan/vulkan_utils.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

static std::map<VkImage, weak_arc<vulkan_image_2d>> s_image_refs;

vulkan_image_2d::vulkan_image_2d(image_specification_t spec, weak_arc<vulkan_logical_device> p_device)
    : m_specification{ std::move(spec) }, m_device{ p_device }, m_descriptor_image_info{}
{
}

vulkan_image_2d::~vulkan_image_2d()
{
    release();
}

void vulkan_image_2d::invalidate()
{
    arc instance{ this };
	render::submit([instance]() mutable
		{
			instance->RT_Invalidate();
		});
}

void vulkan_image_2d::release()
{
	if (!m_info.image)
		return;

    arc instance{ this };
	render::submit_resource_free([device = m_device, info = m_info, layer_views = m_per_layer_image_views]() mutable
		{
			const auto vk_device = device->get_vk_device();
            KB_CORE_INFO("[VulkanImage2D]: destroying image view {}", static_cast<void*>(info.image_view));
			vkDestroyImageView(vk_device, info.image_view, nullptr);
            KB_CORE_INFO("[VulkanImage2D]: destroying sampler {}", static_cast<void*>(info.sampler));
            vk::destroy_sampler(info.sampler);

			for (const auto& view : layer_views)
			{
				if (view)
				{
                    KB_CORE_INFO("[VulkanImage2D]: destroying image view {}", static_cast<void*>(view));
                    vkDestroyImageView(vk_device, view, nullptr);
				}
			}
			vulkan_allocator allocator{ "VulkanImage2D" };
			allocator.DestroyImage(info.image, info.memory_allcation);
			s_image_refs.erase(info.image);
		});

	m_info.image = nullptr;
	m_info.image_view = nullptr;
	m_info.sampler = nullptr;
	m_per_layer_image_views.clear();

	m_mip_image_views.clear();
}

void vulkan_image_2d::RT_Invalidate()
{
    KB_CORE_INFO(
        "[VulkanImage2D]: RT_Invalidate() called for descriptor {}",
        static_cast<void*>(&m_descriptor_image_info)
    );

	// Try release first if necessary
	release();

	VkDevice vk_device = m_device->get_vk_device();
	vulkan_allocator allocator{ "VulkanImage2D" };

	VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT; // TODO: this (probably) shouldn't be implied
	if (m_specification.usage == image_usage_t::Attachment)
	{
		if (backend::util::IsDepthFormat(m_specification.format))
			usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else //if (m_specification.format != ImageFormat::RED32I)
			usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	if (m_specification.m_transfer || m_specification.usage == image_usage_t::Texture)
	{
		usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	if (m_specification.usage == image_usage_t::Storage)
	{
		usage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	VkImageAspectFlags aspect_mask = backend::util::IsDepthFormat(m_specification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	if (m_specification.format == image_format_t::DEPTH24STENCIL8)
		aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;

	VkFormat vulkan_format = util::VulkanImageFormat(m_specification.format);

	VkImageCreateInfo image_create_info = {};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = vulkan_format;
	image_create_info.extent.width = m_specification.width;
	image_create_info.extent.height = m_specification.height;
	image_create_info.extent.depth = 1;
	image_create_info.mipLevels = m_specification.mips;
	image_create_info.arrayLayers = m_specification.layers;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.usage = usage;
	m_info.memory_allcation = allocator.AllocateImage(image_create_info, VMA_MEMORY_USAGE_GPU_ONLY, m_info.image);

	s_image_refs[m_info.image] = this;

	// Create a default image view
	VkImageViewCreateInfo image_view_create_info = {};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create_info.viewType = m_specification.layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
	image_view_create_info.format = vulkan_format;
	image_view_create_info.flags = 0;
	image_view_create_info.subresourceRange = {};
	image_view_create_info.subresourceRange.aspectMask = aspect_mask;
	image_view_create_info.subresourceRange.baseMipLevel = 0;
	image_view_create_info.subresourceRange.levelCount = m_specification.mips;
	image_view_create_info.subresourceRange.baseArrayLayer = 0;
	image_view_create_info.subresourceRange.layerCount = m_specification.layers;
	image_view_create_info.image = m_info.image;

	if (vkCreateImageView(vk_device, &image_view_create_info, nullptr, &m_info.image_view) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "Vulkan failed to create image view!");

	KB_CORE_INFO(
        "[VulkanImage2D]: Created VkImage '{0}' of width '{1}', height '{2}' for descriptor {3}",
        static_cast<void*>(m_info.image),
        m_specification.width,
        m_specification.height,
        static_cast<const void*>(&m_descriptor_image_info)
    );

	// #TODO: Renderer should contain some kind of sampler cache
	VkSamplerCreateInfo sampler_create_info{};
	sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_create_info.maxAnisotropy = 1.0f;
	sampler_create_info.magFilter = VK_FILTER_LINEAR;
	sampler_create_info.minFilter = VK_FILTER_LINEAR;
	sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_create_info.addressModeV = sampler_create_info.addressModeU;
	sampler_create_info.addressModeW = sampler_create_info.addressModeU;
	sampler_create_info.mipLodBias = 0.0f;
	sampler_create_info.maxAnisotropy = 1.0f;
	sampler_create_info.minLod = 0.0f;
	sampler_create_info.maxLod = 100.0f;
	sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    m_info.sampler = vk::create_sampler(sampler_create_info);

	if (m_specification.usage == image_usage_t::Storage)
	{
		// Transition image to GENERAL layout
		VkCommandBuffer command_buffer = m_device->get_vk_command_buffer(true);

		VkImageSubresourceRange subresource_range = {};
		subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource_range.baseMipLevel = 0;
		subresource_range.levelCount = m_specification.mips;
		subresource_range.layerCount = m_specification.layers;

		util::InsertImageMemoryBarrier(
            command_buffer,
            m_info.image,
			0,
            0,
			VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			subresource_range
        );

        m_device->flush_command_buffer(command_buffer);
	}

	UpdateDescriptor();

	KB_CORE_ASSERT(m_descriptor_image_info.imageLayout != VK_IMAGE_LAYOUT_UNDEFINED, "image layout undefined!");
    KB_CORE_ASSERT(m_descriptor_image_info.imageView, "[VulkanImage2D]: Image view is null?");
}

void vulkan_image_2d::create_per_layer_image_views()
{
    arc<vulkan_image_2d> instance{ this };
	render::submit([instance]() mutable
		{
			instance->RT_CreatePerLayerImageViews();
		});

}

void vulkan_image_2d::RT_CreatePerLayerImageViews()
{
	KB_CORE_ASSERT(m_specification.layers > 1, "cannot create per layer image views because there is only one layer!");

    const VkDevice vk_device = m_device->get_vk_device();

	VkImageAspectFlags aspect_mask = render::backend::util::IsDepthFormat(m_specification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	if (m_specification.format == image_format_t::DEPTH24STENCIL8)
		aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;

	const VkFormat vk_format = util::VulkanImageFormat(m_specification.format);

	m_per_layer_image_views.resize(m_specification.layers);
	for (uint32_t layer = 0; layer < m_specification.layers; layer++)
	{
		VkImageViewCreateInfo image_view_create_info = {};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = vk_format;
		image_view_create_info.flags = 0;
		image_view_create_info.subresourceRange = {};
		image_view_create_info.subresourceRange.aspectMask = aspect_mask;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = m_specification.mips;
		image_view_create_info.subresourceRange.baseArrayLayer = layer;
		image_view_create_info.subresourceRange.layerCount = 1;
		image_view_create_info.image = m_info.image;
		if (vkCreateImageView(vk_device, &image_view_create_info, nullptr, &m_per_layer_image_views[layer]) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create per layer image view!");
	}
}

void vulkan_image_2d::RT_CreatePerSpecificLayerImageViews(const std::vector<uint32_t>& layer_indices)
{
    const VkDevice vk_device = m_device->get_vk_device();

	VkImageAspectFlags aspect_mask = render::backend::util::IsDepthFormat(m_specification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	if (m_specification.format == image_format_t::DEPTH24STENCIL8)
		aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;

	const VkFormat vk_format = util::VulkanImageFormat(m_specification.format);

	//HZ_CORE_ASSERT(m_PerLayerImageViews.size() == m_Specification.Layers);
	if (m_per_layer_image_views.empty())
		m_per_layer_image_views.resize(m_specification.layers);

	for (const uint32_t layer : layer_indices)
	{
		VkImageViewCreateInfo image_view_create_info = {};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = vk_format;
		image_view_create_info.flags = 0;
		image_view_create_info.subresourceRange = {};
		image_view_create_info.subresourceRange.aspectMask = aspect_mask;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = m_specification.mips;
		image_view_create_info.subresourceRange.baseArrayLayer = layer;
		image_view_create_info.subresourceRange.layerCount = 1;
		image_view_create_info.image = m_info.image;
		if (vkCreateImageView(vk_device, &image_view_create_info, nullptr, &m_per_layer_image_views[layer]) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan Failed to create image view");
	}
}

VkImageView vulkan_image_2d::GetMipImageView(uint32_t mip)
{
	if (!m_mip_image_views.contains(mip))
	{
        arc<vulkan_image_2d> instance{ this };
		render::submit([instance, mip]() mutable
			{
				instance->RT_GetMipImageView(mip);
			});
		return nullptr;
	}

	return m_mip_image_views.at(mip);

}

VkImageView vulkan_image_2d::RT_GetMipImageView(uint32_t mip)
{
	if (!m_mip_image_views.contains(mip))
	{
        const VkDevice vk_device = m_device->get_vk_device();

		VkImageAspectFlags aspect_mask = render::backend::util::IsDepthFormat(m_specification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (m_specification.format == image_format_t::DEPTH24STENCIL8)
			aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;

        const VkFormat vk_format = util::VulkanImageFormat(m_specification.format);

		m_per_layer_image_views.resize(m_specification.layers);
		VkImageViewCreateInfo image_view_create_info{};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = vk_format;
		image_view_create_info.flags = 0;
		image_view_create_info.subresourceRange = {};
		image_view_create_info.subresourceRange.aspectMask = aspect_mask;
		image_view_create_info.subresourceRange.baseMipLevel = mip;
		image_view_create_info.subresourceRange.levelCount = 1;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = 1;
		image_view_create_info.image = m_info.image;

		if (vkCreateImageView(vk_device, &image_view_create_info, nullptr, &m_mip_image_views[mip]) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create mipmap image views!");
	}
	return m_mip_image_views.at(mip);

}

void vulkan_image_2d::UpdateDescriptor()
{
    KB_CORE_INFO("[VulkanImage2D]: Updating descriptor {}", static_cast<const void*>(&m_descriptor_image_info));
	if (m_specification.format == image_format_t::DEPTH24STENCIL8 || m_specification.format == image_format_t::DEPTH32F)
		m_descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	else if (m_specification.usage == image_usage_t::Storage)
		m_descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	else
		m_descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	m_descriptor_image_info.imageView = m_info.image_view;
	m_descriptor_image_info.sampler = m_info.sampler;
}

const std::map<VkImage, weak_arc<vulkan_image_2d>>& vulkan_image_2d::GetImageRefs() const
{
	return s_image_refs;
}

// --- vulkan_image_view -----------------------------

vulkan_image_view::vulkan_image_view(image_view_specification p_specification, weak_arc<vulkan_logical_device> p_device)
    : m_specification{ std::move(p_specification) }, m_device{ p_device }
{
}

vulkan_image_view::~vulkan_image_view()
{
    render::submit_resource_free([device = m_device, image_view = m_vk_image_view]() mutable
        {
            const auto vk_device = device->get_vk_device();
            vkDestroyImageView(vk_device, image_view, nullptr);
        });

    m_vk_image_view = nullptr;
}


auto vulkan_image_view::invalidate() noexcept -> void
{
    arc instance{ this };
    render::submit([instance]() mutable
        {
            instance->rt_invalidate();
        });
}

auto vulkan_image_view::rt_invalidate() noexcept -> void
{
    auto vulkan_image = m_specification.m_image.As<vulkan_image_2d>();
    const auto& image_spec = vulkan_image->get_specification();

    VkImageAspectFlags aspect_mask = render::backend::util::IsDepthFormat(image_spec.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    if (image_spec.format == image_format_t::DEPTH24STENCIL8)
        aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;

    const auto vk_format = util::VulkanImageFormat(image_spec.format);

    const VkImageViewCreateInfo image_view_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = vulkan_image->get_vk_image_info().image,
        .viewType = image_spec.layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D,
        .format = vk_format,
        .components = {},
        .subresourceRange = VkImageSubresourceRange{
            .aspectMask = aspect_mask,
            .baseMipLevel = m_specification.m_mip,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = image_spec.layers
        },
    };

    const auto vk_device = m_device->get_vk_device();
    KB_VK_CHECK_RESULT(
        vkCreateImageView(
            vk_device,
            &image_view_create_info,
            nullptr,
            &m_vk_image_view
        )
    )

    m_vk_descriptor_image_info = vulkan_image->get_vk_image_info_descriptor();
    m_vk_descriptor_image_info.imageView = m_vk_image_view;
}

// ---------------------------------------------------

} // end namespace kb::render::backend::vk
