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
		OpenGLFramebuffer(const FrameBufferSpecification& specs);
		virtual ~OpenGLFramebuffer();

		void Invalidate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual uint32_t GetColorAttachmentRendererID() const override { return m_color_attachment; }

		virtual const FrameBufferSpecification& GetSpecification() const override { return m_specifications; }
	private:
		Renderer::RendererID m_renderer_id;
		uint32_t m_color_attachment, m_depth_attachment;
		FrameBufferSpecification m_specifications;
	};
}

#endif
