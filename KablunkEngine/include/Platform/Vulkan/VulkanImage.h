#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_IMAGE_H
#define KABLUNK_PLATFORM_VULKAN_IMAGE_H

#include "Kablunk/Renderer/Image.h"
#include "Platform/Vulkan/VulkanAllocator.h"
#include "Platform/Vulkan/VulkanContext.h"

#include <vulkan/vulkan.h>

#include <map>

namespace kb
{ // start namespace kb
struct VulkanImageInfo
{
	VkImage image = nullptr;
	VkImageView image_view = nullptr;
	VkSampler sampler = nullptr;
	VmaAllocation memory_allcation = nullptr;
};


class VulkanImage2D final : public Image2D
{
public:
	VulkanImage2D(ImageSpecification spec);
	~VulkanImage2D() override;

	void Invalidate() override;
	void Release() override;

	uint32_t GetWidth() const override { return m_specification.width; }
	uint32_t GetHeight() const override { return m_specification.height; }
    glm::uvec2 get_size() const override { return glm::uvec2{ m_specification.width, m_specification.height }; }
	float GetAspectRatio() const override { return static_cast<float>(m_specification.width) / static_cast<float>(m_specification.height); }

	ImageSpecification& GetSpecification() override { return m_specification; }
	const ImageSpecification& GetSpecification() const override { return m_specification; }

	void RT_Invalidate();

	const std::map<VkImage, WeakRef<VulkanImage2D>>& GetImageRefs() const;

	void CreatePerLayerImageViews() override;
	void RT_CreatePerLayerImageViews();
	void RT_CreatePerSpecificLayerImageViews(const std::vector<uint32_t>& layer_indices);

	VkImageView GetLayerImageView(uint32_t layer)
	{
		KB_CORE_ASSERT(layer < m_per_layer_image_views.size(), "out of bounds!");
		return m_per_layer_image_views[layer];
	}

	VkImageView GetMipImageView(uint32_t mip);
	VkImageView RT_GetMipImageView(uint32_t mip);

	VulkanImageInfo& get_vk_image_info() { return m_info; }
	const VulkanImageInfo& get_vk_image_info() const { return m_info; }

    resource_descriptor_info_t get_descriptor_info() noexcept override
	{
        return &m_descriptor_image_info;
	}

	const VkDescriptorImageInfo& get_vk_image_info_descriptor() const { return m_descriptor_image_info; }

	const owning_buffer& GetBuffer() const override { return m_image_data; }
	owning_buffer& GetBuffer() override { return m_image_data; }

	uint64_t GetHash() const override { return reinterpret_cast<uint64_t>(m_info.image); }

	void UpdateDescriptor();

private:
	ImageSpecification m_specification;
	VulkanImageInfo m_info;

	VkDescriptorImageInfo m_descriptor_image_info;
    // local buffer for the image, deleted after the image is transfered to the gpu
	owning_buffer m_image_data;

	std::vector<VkImageView> m_per_layer_image_views;
	std::map<uint32_t, VkImageView> m_mip_image_views;
};

class vulkan_image_view final : public image_view
{
public:
    vulkan_image_view(image_view_specification p_specification);
    ~vulkan_image_view() override;

    auto invalidate() noexcept -> void;
    auto rt_invalidate() noexcept -> void;

    auto get_vk_image_view() const noexcept -> VkImageView { return m_vk_image_view; }

    resource_descriptor_info_t get_descriptor_info() noexcept override
    {
        return &m_vk_image_view;
    }

    auto get_vk_descriptor_image_info() const noexcept -> const VkDescriptorImageInfo&
    {
        return m_vk_descriptor_image_info;
    }
private:
    image_view_specification m_specification{};
    VkImageView m_vk_image_view{};
    VkDescriptorImageInfo m_vk_descriptor_image_info{};
};

namespace Utils
{ // start namespace ::Utils
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
} // end namespace ::Utils

} // end namespace kb

#endif
