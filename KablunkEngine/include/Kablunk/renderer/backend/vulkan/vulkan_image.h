#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_IMAGE_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_IMAGE_H

#include "Kablunk/Renderer/backend/image.h"
#include "kablunk/renderer/backend/vulkan/vulkan_allocator.h"
#include "kablunk/renderer/backend/vulkan/vulkan_context.h"

#include <vulkan/vulkan.h>

#include <map>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

struct VulkanImageInfo
{
	VkImage image = nullptr;
	VkImageView image_view = nullptr;
	VkSampler sampler = nullptr;
	VmaAllocation memory_allcation = nullptr;
};

class vulkan_image_2d final : public image_2d
{
public:
	vulkan_image_2d(image_specification_t spec, weak_ptr<vulkan_logical_device> p_device);
	~vulkan_image_2d() override;

	void invalidate() override;
	void release() override;

	uint32_t get_width() const override { return m_specification.width; }
	uint32_t get_height() const override { return m_specification.height; }
    glm::uvec2 get_size() const override { return glm::uvec2{ m_specification.width, m_specification.height }; }
	float get_aspect_ratio() const override { return static_cast<float>(m_specification.width) / static_cast<float>(m_specification.height); }

	image_specification_t& get_specification() override { return m_specification; }
	const image_specification_t& get_specification() const override { return m_specification; }

	void RT_Invalidate();

	const std::map<VkImage, weak_ptr<vulkan_image_2d>>& GetImageRefs() const;

	void create_per_layer_image_views() override;
	void RT_CreatePerLayerImageViews();
	void RT_CreatePerSpecificLayerImageViews(const std::vector<uint32_t>& layer_indices);

	VkImageView get_layer_vk_image_view(uint32_t layer) const noexcept
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

	const owning_buffer& get_buffer() const override { return m_image_data; }
	owning_buffer& get_buffer() override { return m_image_data; }

	uint64_t get_hash() const override { return reinterpret_cast<uint64_t>(m_info.image); }


private:
    void update_vk_descriptor_image_info();

	image_specification_t m_specification;
    weak_ptr<vulkan_logical_device> m_device;

    // TODO: why duplicated?
	VulkanImageInfo m_info;
	VkDescriptorImageInfo m_descriptor_image_info;

    // local buffer for the image, deleted after the image is transfered to the gpu
	owning_buffer m_image_data;

	std::vector<VkImageView> m_per_layer_image_views;
	std::map<uint32_t, VkImageView> m_mip_image_views;

    friend class vulkan_texture_2d;
};

class vulkan_image_view final : public image_view
{
public:
    vulkan_image_view(image_view_specification p_specification, weak_ptr<vulkan_logical_device> p_device);
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
    weak_ptr<vulkan_logical_device> m_device{};
    VkImageView m_vk_image_view{};
    VkDescriptorImageInfo m_vk_descriptor_image_info{};
};

} // end namespace kb::render::backend::vk

#endif
