#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_IMAGE_H
#define KABLUNK_PLATFORM_VULKAN_IMAGE_H

#include "Kablunk/Renderer/Image.h"
#include "Platform/Vulkan/VulkanAllocator.h"
#include "Platform/Vulkan/VulkanContext.h"

#include <vulkan/vulkan.h>

#include <map>

namespace Kablunk
{
	struct VulkanImageInfo
	{
		VkImage image = nullptr;
		VkImageView image_view = nullptr;
		VkSampler sampler = nullptr;
		VmaAllocation memory_allcation = nullptr;
	};


	class VulkanImage2D : public Image2D
	{
	public:
		VulkanImage2D(ImageSpecification spec);
		virtual ~VulkanImage2D() override;

		virtual void Invalidate() override;
		virtual void Release() override;

		virtual uint32_t GetWidth() const override { return m_specification.width; }
		virtual uint32_t GetHeight() const override { return m_specification.height; }
		virtual float GetAspectRatio() const override { return static_cast<float>(m_specification.width) / static_cast<float>(m_specification.height); }

		virtual ImageSpecification& GetSpecification() override { return m_specification; }
		virtual const ImageSpecification& GetSpecification() const override { return m_specification; }

		void RT_Invalidate();

		const std::map<VkImage, WeakRef<VulkanImage2D>>& GetImageRefs() const;

		virtual void CreatePerLayerImageViews() override;
		void RT_CreatePerLayerImageViews();
		void RT_CreatePerSpecificLayerImageViews(const std::vector<uint32_t>& layer_indices);

		virtual VkImageView GetLayerImageView(uint32_t layer)
		{
			KB_CORE_ASSERT(layer < m_per_layer_image_views.size(), "out of bounds!");
			return m_per_layer_image_views[layer];
		}

		VkImageView GetMipImageView(uint32_t mip);
		VkImageView RT_GetMipImageView(uint32_t mip);

		VulkanImageInfo& GetImageInfo() { return m_info; }
		const VulkanImageInfo& GetImageInfo() const { return m_info; }

		const VkDescriptorImageInfo& GetDescriptor() { return m_descriptor_image_info; }

		virtual const Buffer& GetBuffer() const override { return m_image_data; }
		virtual Buffer& GetBuffer() override { return m_image_data; }

		virtual uint64_t GetHash() const override { return (uint64_t)(m_info.image); }

		void UpdateDescriptor();
	private:
		ImageSpecification m_specification;
		VulkanImageInfo m_info;

		VkDescriptorImageInfo m_descriptor_image_info;

		Buffer m_image_data;

		std::vector<VkImageView> m_per_layer_image_views;
		std::map<uint32_t, VkImageView> m_mip_image_views;
		
	};

	namespace Utils {

		inline VkFormat VulkanImageFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RED32I:		   return VK_FORMAT_R32_SINT;
			case ImageFormat::RED32F:          return VK_FORMAT_R32_SFLOAT;
			case ImageFormat::RG16F:		   return VK_FORMAT_R16G16_SFLOAT;
			case ImageFormat::RG32F:		   return VK_FORMAT_R32G32_SFLOAT;
			case ImageFormat::RGBA:            return VK_FORMAT_R8G8B8A8_UNORM;
			case ImageFormat::RGBA16F:         return VK_FORMAT_R16G16B16A16_SFLOAT;
			case ImageFormat::RGBA32F:         return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ImageFormat::DEPTH32F:        return VK_FORMAT_D32_SFLOAT;
			case ImageFormat::DEPTH24STENCIL8: return VulkanContext::Get()->GetDevice()->GetPhysicalDevice()->GetDepthFormat();
			}
			KB_CORE_ASSERT(false, "Unknown ImageFormat!");
			return VK_FORMAT_UNDEFINED;
		}

	}

}

#endif
