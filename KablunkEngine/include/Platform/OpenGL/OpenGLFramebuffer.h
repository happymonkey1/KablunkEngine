#ifndef PLATFORM_OPENGL_OPENGLFRAMEBUFFER_H
#define PLATFORM_OPENGL_OPENGLFRAMEBUFFER_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Framebuffer.h"
#include "Kablunk/Renderer/Renderer.h"

namespace kb
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& specs);
		virtual ~OpenGLFramebuffer();

		virtual void AddResizeCallback(const std::function<void(ref<Framebuffer>)>& func) override;
		void Invalidate();

		virtual void Bind() const override;
		virtual void Unbind() const  override;

		virtual void BindTexture(uint32_t attachment_index = 0, uint32_t slot = 0) const override;
		virtual uint32_t GetWidth() const { return m_specification.width; };
		virtual uint32_t GetHeight() const override { return m_specification.height; };


		virtual void Resize(uint32_t width, uint32_t height, bool force_recreate = false) override;
		virtual int ReadPixel(uint32_t attachment_index, int x, int y) override;

		virtual void ClearAttachment(uint32_t attachment_index, int value) override;

		virtual RendererID GetRendererID() const { return m_renderer_id; }
		uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const { KB_CORE_ASSERT(index < m_color_attachments.size(), "index out of bounds!"); return m_color_attachments[index]; }
		RendererID GetDepthAttachmentRendererID() const { return m_depth_attachment; };

		virtual ref<Image2D> GetImage(uint32_t attachment_index = 0) const override;
		virtual ref<Image2D> GetDepthImage() const override;

		virtual const FramebufferSpecification& GetSpecification() const override { return m_specification; }
	private:

		void DeleteBuffer();

		RendererID m_renderer_id;
		FramebufferSpecification m_specification;
		
		std::vector<FramebufferTextureSpecification> m_color_attachment_specs;
		FramebufferTextureSpecification m_depth_attachment_spec = FramebufferTextureSpecification{ ImageFormat::RGBA };

		// Renderer IDs
		std::vector<RendererID> m_color_attachments;
		RendererID m_depth_attachment;
	};
}

#endif
