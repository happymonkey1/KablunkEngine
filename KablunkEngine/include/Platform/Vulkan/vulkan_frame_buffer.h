#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_FRAMEBUFFER_H
#define KABLUNK_PLATFORM_VULKAN_FRAMEBUFFER_H

#include "Kablunk/Renderer/RendererTypes.h"
#include "Kablunk/Renderer/frame_buffer.h"

#include "Kablunk/Renderer/Texture.h"

#include "Platform/Vulkan/VulkanImage.h"

#include <vulkan/vulkan.h>

namespace kb::render
{ // start namespace kb::render
class vulkan_frame_buffer final : public frame_buffer 
{
public:
	vulkan_frame_buffer(frame_buffer_specification spec);
	virtual ~vulkan_frame_buffer() override;

	virtual void resize(uint32_t width, uint32_t height, bool forceRecreate = false) override;
	virtual void add_resize_callback(const std::function<void(arc<frame_buffer>)>& func) override;

	virtual void bind() const override {}
	virtual void unbind() const override {}

	virtual void bind_texture(uint32_t attachmentIndex = 0, uint32_t slot = 0) const override {}

	virtual uint32_t get_width() const override { return m_width; }
	virtual uint32_t get_height() const override { return m_height; }
	virtual RendererID get_renderer_id() const { return 0; }
	virtual RendererID GetColorAttachmentRendererID() const { return 0; }
	virtual RendererID GetDepthAttachmentRendererID() const { return 0; }

	virtual arc<Image2D> get_image(uint32_t attachment_index = 0) const override
	{ 
		KB_CORE_ASSERT(attachment_index < m_attachment_images.size(), "out of bounds!"); 
		//KB_CORE_ASSERT(m_attachment_images[attachment_index].As<VulkanImage2D>()->get_vk_image_info_descriptor().imageLayout != VK_IMAGE_LAYOUT_UNDEFINED, "layout undefined!");

		return m_attachment_images[attachment_index];
	}

	virtual arc<Image2D> get_depth_image() const override { return m_depth_attachment_image; }

	virtual int read_pixel(uint32_t attachment_index, int x, int y) override;
	virtual void clear_attachment(uint32_t attachment_index, int value) override;

	size_t GetColorAttachmentCount() const { return m_specification.m_swap_chain_target ? 1 : m_attachment_images.size(); }
	bool HasDepthAttachment() const { return (bool)m_depth_attachment_image; }
	VkRenderPass GetVkRenderPass() const { return m_vk_render_pass; }
	VkFramebuffer GetVkFramebuffer() const { return m_framebuffer; }
	const std::vector<VkClearValue>& GetVkClearValues() const { return m_clear_values; }

	virtual const frame_buffer_specification& get_specification() const override { return m_specification; }

    [[nodiscard]] u32 get_color_attachment_count() const noexcept override
	{
	    return m_specification.m_swap_chain_target ? 1 : m_attachment_images.size();
	}

    [[nodiscard]] bool has_depth_attachment() const noexcept override
	{
	    return m_depth_attachment_image != arc<Image2D>{};
	}

	void Invalidate();
	void RT_Invalidate();
private:
	frame_buffer_specification m_specification;
	uint32_t m_width = 0, m_height = 0;

	std::vector<arc<Image2D>> m_attachment_images;
	arc<Image2D> m_depth_attachment_image;

	std::vector<VkClearValue> m_clear_values;
	VkRenderPass m_vk_render_pass = nullptr;
	VkFramebuffer m_framebuffer = nullptr;

	std::vector<std::function<void(arc<frame_buffer>)>> m_resize_callbacks;

};
} // end namespace kb::render

#endif
