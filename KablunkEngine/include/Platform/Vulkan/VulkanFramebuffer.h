#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_FRAMEBUFFER_H
#define KABLUNK_PLATFORM_VULKAN_FRAMEBUFFER_H

#include "Kablunk/Renderer/RendererTypes.h"
#include "Kablunk/Renderer/Framebuffer.h"

#include "Kablunk/Renderer/Texture.h"

#include "Platform/Vulkan/VulkanImage.h"

#include <vulkan/vulkan.h>

namespace Kablunk
{
	class VulkanFramebuffer : public Framebuffer 
	{
	public:
		VulkanFramebuffer(const FramebufferSpecification& spec);
		virtual ~VulkanFramebuffer() override;

		virtual void Resize(uint32_t width, uint32_t height, bool forceRecreate = false) override;
		virtual void AddResizeCallback(const std::function<void(IntrusiveRef<Framebuffer>)>& func) override;

		virtual void Bind() const override {}
		virtual void Unbind() const override {}

		virtual void BindTexture(uint32_t attachmentIndex = 0, uint32_t slot = 0) const override {}

		virtual uint32_t GetWidth() const override { return m_width; }
		virtual uint32_t GetHeight() const override { return m_height; }
		virtual RendererID GetRendererID() const { return 0; }
		virtual RendererID GetColorAttachmentRendererID() const { return 0; }
		virtual RendererID GetDepthAttachmentRendererID() const { return 0; }

		virtual IntrusiveRef<Image2D> GetImage(uint32_t attachment_index = 0) const override
		{ 
			KB_CORE_ASSERT(attachment_index < m_attachment_images.size(), "out of bounds!"); 
			//KB_CORE_ASSERT(m_attachment_images[attachment_index].As<VulkanImage2D>()->GetDescriptor().imageLayout != VK_IMAGE_LAYOUT_UNDEFINED, "layout undefined!");

			return m_attachment_images[attachment_index];
		}
		virtual IntrusiveRef<Image2D> GetDepthImage() const override { return m_depth_attachment_image; }

		virtual int ReadPixel(uint32_t attachment_index, int x, int y) override;
		virtual void ClearAttachment(uint32_t attachment_index, int value) override;

		size_t GetColorAttachmentCount() const { return m_specification.swap_chain_target ? 1 : m_attachment_images.size(); }
		bool HasDepthAttachment() const { return (bool)m_depth_attachment_image; }
		VkRenderPass GetVkRenderPass() const { return m_render_pass; }
		VkFramebuffer GetVkFramebuffer() const { return m_framebuffer; }
		const std::vector<VkClearValue>& GetVkClearValues() const { return m_clear_values; }

		virtual const FramebufferSpecification& GetSpecification() const override { return m_specification; }

		void Invalidate();
		void RT_Invalidate();
	private:
		FramebufferSpecification m_specification;
		uint32_t m_width = 0, m_height = 0;

		std::vector<IntrusiveRef<Image2D>> m_attachment_images;
		IntrusiveRef<Image2D> m_depth_attachment_image;

		std::vector<VkClearValue> m_clear_values;
		VkRenderPass m_render_pass = nullptr;
		VkFramebuffer m_framebuffer = nullptr;

		std::vector<std::function<void(IntrusiveRef<Framebuffer>)>> m_resize_callbacks;

	};
}

#endif
