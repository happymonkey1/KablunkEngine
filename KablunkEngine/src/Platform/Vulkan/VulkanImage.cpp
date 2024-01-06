#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanImage.h"

#include "Kablunk/Renderer/RenderCommand.h"
#include "Platform/Vulkan/VulkanRenderer.h"

namespace kb
{
	static std::map<VkImage, WeakRef<VulkanImage2D>> s_image_refs;

	VulkanImage2D::VulkanImage2D(ImageSpecification spec)
		: m_specification{ spec }, m_descriptor_image_info{}
	{

	}

	VulkanImage2D::~VulkanImage2D()
	{

		if (m_info.image)
		{
			const VulkanImageInfo& info = m_info;
			render::submit_resource_free([info, layer_views = m_per_layer_image_views]()
				{
					const auto vk_device = VulkanContext::Get()->GetDevice()->GetVkDevice();
					vkDestroyImageView(vk_device, info.image_view, nullptr);
					vkDestroySampler(vk_device, info.sampler, nullptr);

					for (auto& view : layer_views)
						if (view)
							vkDestroyImageView(vk_device, view, nullptr);

					VulkanAllocator allocator{ "VulkanImage2D" };
					allocator.DestroyImage(info.image, info.memory_allcation);
					s_image_refs.erase(info.image);
				});

			m_per_layer_image_views.clear();
		}

	}

	void VulkanImage2D::Invalidate()
	{
        ref<VulkanImage2D> instance{ this };
		render::submit([instance]() mutable
			{
				instance->RT_Invalidate();
			});

	}

	void VulkanImage2D::Release()
	{
		if (!m_info.image)
			return;

        ref<VulkanImage2D> instance{ this };
		render::submit_resource_free([info = m_info, layer_views = m_per_layer_image_views]() mutable
			{
				const auto vk_device = VulkanContext::Get()->GetDevice()->GetVkDevice();
				vkDestroyImageView(vk_device, info.image_view, nullptr);
				vkDestroySampler(vk_device, info.sampler, nullptr);

				for (auto& view : layer_views)
				{
					if (view)
						vkDestroyImageView(vk_device, view, nullptr);
				}
				VulkanAllocator allocator{ "VulkanImage2D" };
				allocator.DestroyImage(info.image, info.memory_allcation);
				s_image_refs.erase(info.image);
			});

		m_info.image = nullptr;
		m_info.image_view = nullptr;
		m_info.sampler = nullptr;
		m_per_layer_image_views.clear();

		m_mip_image_views.clear();
	}

	void VulkanImage2D::RT_Invalidate()
	{
		// Try release first if necessary
		Release();

		VkDevice vk_device = VulkanContext::Get()->GetDevice()->GetVkDevice();
		VulkanAllocator allocator{ "VulkanImage2D" };

		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT; // TODO: this (probably) shouldn't be implied
		if (m_specification.usage == ImageUsage::Attachment)
		{
			if (Utils::IsDepthFormat(m_specification.format))
				usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			else //if (m_specification.format != ImageFormat::RED32I)
				usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
		else if (m_specification.usage == ImageUsage::Texture)
		{
			usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		else if (m_specification.usage == ImageUsage::Storage)
		{
			usage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		VkImageAspectFlags aspect_mask = Utils::IsDepthFormat(m_specification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (m_specification.format == ImageFormat::DEPTH24STENCIL8)
			aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		VkFormat vulkan_format = Utils::VulkanImageFormat(m_specification.format);

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

		KB_CORE_INFO("Created VkImage '{0}' of width '{1}', height '{2}'", (void*)m_info.image, m_specification.width, m_specification.height);

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
		if (vkCreateSampler(vk_device, &sampler_create_info, nullptr, &m_info.sampler) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create sampler!");

		if (m_specification.usage == ImageUsage::Storage)
		{
			// Transition image to GENERAL layout
			VkCommandBuffer command_buffer = VulkanContext::Get()->GetDevice()->GetCommandBuffer(true);

			VkImageSubresourceRange subresource_range = {};
			subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresource_range.baseMipLevel = 0;
			subresource_range.levelCount = m_specification.mips;
			subresource_range.layerCount = m_specification.layers;

			Utils::InsertImageMemoryBarrier(command_buffer, m_info.image,
				0, 0,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				subresource_range);

			VulkanContext::Get()->GetDevice()->FlushCommandBuffer(command_buffer);
		}

		UpdateDescriptor();

		KB_CORE_ASSERT(m_descriptor_image_info.imageLayout != VK_IMAGE_LAYOUT_UNDEFINED, "image layout undefined!");
	}

	void VulkanImage2D::CreatePerLayerImageViews()
	{
        ref<VulkanImage2D> instance{ this };
		render::submit([instance]() mutable
			{
				instance->RT_CreatePerLayerImageViews();
			});

	}

	void VulkanImage2D::RT_CreatePerLayerImageViews()
	{
		KB_CORE_ASSERT(m_specification.layers > 1, "cannot create per layer image views because there is only one layer!");

		VkDevice vk_device = VulkanContext::Get()->GetDevice()->GetVkDevice();

		VkImageAspectFlags aspect_mask = Utils::IsDepthFormat(m_specification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (m_specification.format == ImageFormat::DEPTH24STENCIL8)
			aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		const VkFormat vk_format = Utils::VulkanImageFormat(m_specification.format);

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

	void VulkanImage2D::RT_CreatePerSpecificLayerImageViews(const std::vector<uint32_t>& layer_indices)
	{
		VkDevice vk_device = VulkanContext::Get()->GetDevice()->GetVkDevice();

		VkImageAspectFlags aspect_mask = Utils::IsDepthFormat(m_specification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (m_specification.format == ImageFormat::DEPTH24STENCIL8)
			aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		const VkFormat vk_format = Utils::VulkanImageFormat(m_specification.format);

		//HZ_CORE_ASSERT(m_PerLayerImageViews.size() == m_Specification.Layers);
		if (m_per_layer_image_views.empty())
			m_per_layer_image_views.resize(m_specification.layers);

		for (uint32_t layer : layer_indices)
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

	VkImageView VulkanImage2D::GetMipImageView(uint32_t mip)
	{
		if (m_mip_image_views.find(mip) == m_mip_image_views.end())
		{
            ref<VulkanImage2D> instance{ this };
			render::submit([instance, mip]() mutable
				{
					instance->RT_GetMipImageView(mip);
				});
			return nullptr;
		}

		return m_mip_image_views.at(mip);

	}

	VkImageView VulkanImage2D::RT_GetMipImageView(uint32_t mip)
	{
		if (m_mip_image_views.find(mip) == m_mip_image_views.end())
		{
			VkDevice vk_device = VulkanContext::Get()->GetDevice()->GetVkDevice();

			VkImageAspectFlags aspect_mask = Utils::IsDepthFormat(m_specification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
			if (m_specification.format == ImageFormat::DEPTH24STENCIL8)
				aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;

			VkFormat vk_format = Utils::VulkanImageFormat(m_specification.format);

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

	void VulkanImage2D::UpdateDescriptor()
	{
		if (m_specification.format == ImageFormat::DEPTH24STENCIL8 || m_specification.format == ImageFormat::DEPTH32F)
			m_descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		else if (m_specification.usage == ImageUsage::Storage)
			m_descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		else
			m_descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		m_descriptor_image_info.imageView = m_info.image_view;
		m_descriptor_image_info.sampler = m_info.sampler;
	}

	const std::map<VkImage, WeakRef<VulkanImage2D>>& VulkanImage2D::GetImageRefs() const
	{
		return s_image_refs;
	}

}
