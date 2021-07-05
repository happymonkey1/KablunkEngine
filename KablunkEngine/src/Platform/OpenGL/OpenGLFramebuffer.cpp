#include "kablunkpch.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace Kablunk
{

	static const uint32_t s_max_frame_buffer_size{ 8192 }; // Should be determined by gpu capabilities during runtime 

	OpenGLFramebuffer::OpenGLFramebuffer(const FrameBufferSpecification& specs)
		: m_specifications{ specs }, m_renderer_id{ 0 }, m_color_attachment{ 0 }, m_depth_attachment{ 0 }
	{
		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		DeleteBuffer();
	}
	void OpenGLFramebuffer::Invalidate()
	{
		if (m_renderer_id)
			DeleteBuffer();

		glCreateFramebuffers(1, &m_renderer_id);
		glBindFramebuffer(GL_FRAMEBUFFER, m_renderer_id);

		// Color
		glCreateTextures(GL_TEXTURE_2D, 1, &m_color_attachment);
		glBindTexture   (GL_TEXTURE_2D, m_color_attachment);
		glTexImage2D    (GL_TEXTURE_2D, 0, GL_RGBA8, m_specifications.width, m_specifications.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_attachment, 0);

		// Depth 
		glCreateTextures(GL_TEXTURE_2D, 1, &m_depth_attachment);
		glBindTexture   (GL_TEXTURE_2D, m_depth_attachment);
		glTexStorage2D  (GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_specifications.width, m_specifications.height);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_attachment, 0);

		KB_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer incomplete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_renderer_id);
		glViewport(0, 0, m_specifications.width, m_specifications.height);
	}

	void OpenGLFramebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > s_max_frame_buffer_size || height > s_max_frame_buffer_size)
		{
			KB_CORE_WARN("Attempt to resize framebuffer to invalid size, ({0}, {1})", width, height);
			return;
		}
		m_specifications.width = width;
		m_specifications.height = height;

		Invalidate();
	}

	void OpenGLFramebuffer::DeleteBuffer()
	{
		glDeleteFramebuffers(1, &m_renderer_id);
		glDeleteTextures(1, &m_color_attachment);
		glDeleteTextures(1, &m_depth_attachment);
	}

}
