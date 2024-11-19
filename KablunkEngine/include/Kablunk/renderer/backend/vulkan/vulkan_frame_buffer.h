#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_FRAMEBUFFER_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_FRAMEBUFFER_H

#include "Kablunk/renderer/backend/frame_buffer.h"
#include "Kablunk/renderer/backend/image.h"
#include "kablunk/renderer/backend/vulkan/vulkan_image.h"

#include <vulkan/vulkan.h>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

class vulkan_frame_buffer final : public frame_buffer
{
public:
	vulkan_frame_buffer(
        VkDevice p_vk_device,
        frame_buffer_specification_t spec
    );
	~vulkan_frame_buffer() override;

	void resize(u32 p_width, u32 p_height, bool p_force_recreate = false) override;
	void add_resize_callback(const std::function<void(arc<frame_buffer>)>& func) override;

	void bind() const override {}
	void unbind() const override {}

	void bind_texture(u32 p_attachment_index = 0, u32 p_slot = 0) const override {}

    u32 get_width() const override { return m_width; }
	u32 get_height() const override { return m_height; }

	arc<image_2d> get_image(const u32 p_attachment_index = 0) const override
	{
		KB_CORE_ASSERT(p_attachment_index < m_attachment_images.size(), "out of bounds!");
		//KB_CORE_ASSERT(m_attachment_images[attachment_index].As<VulkanImage2D>()->get_vk_image_info_descriptor().imageLayout != VK_IMAGE_LAYOUT_UNDEFINED, "layout undefined!");

		return m_attachment_images[p_attachment_index];
	}

	arc<image_2d> get_depth_image() const override { return m_depth_attachment_image; }

	int read_pixel(u32 p_attachment_index, int p_x, int p_y) override;
	void clear_attachment(u32 p_attachment_index, int p_value) override;

	bool has_depth_attachment() const noexcept override
	{
	    return m_depth_attachment_image != arc<image_2d>{};
	}

    // Retrieve the vulkan render pass
	VkRenderPass get_vk_render_pass() const { return m_vk_render_pass; }
    // Retrieve the vulkan framebuffer
	VkFramebuffer get_vk_frame_buffer() const { return m_vk_frame_buffer; }
    // Retrieve the vulkan clear values
	const std::vector<VkClearValue>& get_vk_clear_values() const { return m_clear_values; }

	const frame_buffer_specification_t& get_specification() const override { return m_specification; }

    [[nodiscard]] u32 get_color_attachment_count() const noexcept override
	{
	    return m_specification.m_swap_chain_target ? 1 : static_cast<u32>(m_attachment_images.size());
	}

    // Invalidate frame buffer object, scheduling invalidation on the render thread as a task
	void invalidate();

    // Invalidation logic for the frame buffer
    // Should only be run on render thread
	void rt_invalidate();
private:
	frame_buffer_specification_t m_specification;
	u32 m_width = 0, m_height = 0;
    VkDevice m_vk_device = nullptr;

	std::vector<arc<image_2d>> m_attachment_images;
	arc<image_2d> m_depth_attachment_image;

	std::vector<VkClearValue> m_clear_values;
	VkRenderPass m_vk_render_pass = nullptr;
	VkFramebuffer m_vk_frame_buffer = nullptr;

	std::vector<std::function<void(arc<frame_buffer>)>> m_resize_callbacks;
};

} // end namespace kb::render::backend::vk

#endif
