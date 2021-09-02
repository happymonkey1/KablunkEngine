#ifndef PLATFORM_OPENGL_OPENGLFRAMEBUFFER_H
#define PLATFORM_OPENGL_OPENGLFRAMEBUFFER_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Framebuffer.h"
#include "Kablunk/Renderer/Renderer.h"

namespace Kablunk
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& specs);
		virtual ~OpenGLFramebuffer();

		void Invalidate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual int ReadPixel(uint32_t attachment_index, int x, int y) override;

		virtual void ClearAttachment(uint32_t attachment_index, int value) override;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override { KB_CORE_ASSERT(index < m_color_attachments.size(), "index out of bounds!"); return m_color_attachments[index]; }

		virtual const FramebufferSpecification& GetSpecification() const override { return m_specification; }
	private:

		void DeleteBuffer();

		RendererID m_renderer_id;
		FramebufferSpecification m_specification;
		
		std::vector<FramebufferTextureSpecification> m_color_attachment_specs;
		FramebufferTextureSpecification m_depth_attachment_spec = FramebufferTextureFormat::None;

		// Renderer IDs
		std::vector<RendererID> m_color_attachments;
		RendererID m_depth_attachment;
	};
}

#endif
