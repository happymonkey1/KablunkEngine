#include "kablunkpch.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace Kablunk
{

	static const uint32_t s_max_frame_buffer_size{ 8192 }; // Should be determined by gpu capabilities during runtime 

	namespace Utilities
	{
		static GLenum TextureTarget(bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool multisampled, uint32_t* out_id, uint32_t count)
		{
			glCreateTextures(TextureTarget(multisampled), count, out_id);
		}

		static void BindTexture(bool multisampled, Renderer::RendererID id)
		{
			glBindTexture(TextureTarget(multisampled), id);
		}

		static void AttachColorTexture(Renderer::RendererID id, int samples, GLenum internal_format, GLenum access_format, uint32_t width, uint32_t height, int index)
		{
			bool multisampled = samples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internal_format, width, height, GL_FALSE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, access_format, GL_UNSIGNED_BYTE, nullptr);

				// #TODO set based off framebuffer specification
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
		}

		static void AttachDepthTexture(Renderer::RendererID id, int samples, GLenum format, GLenum attachment_type, uint32_t width, uint32_t height)
		{
			bool multisampled = samples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else
			{
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

				// #TODO set based off framebuffer specification
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_type, TextureTarget(multisampled), id, 0);
		}

		static bool IsDepthFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8: return true;
			}

			return false;
		}

		static GLenum KablunkTextureFormatToGLenum(FramebufferTextureFormat format)
		{
			switch (format)
			{
				case FramebufferTextureFormat::RGBA8:		return GL_RGBA8;
				case FramebufferTextureFormat::RED_INTEGER: return GL_RED_INTEGER;
			}

			KB_CORE_ASSERT(false, "Unknown format");
			return 0;
		}
	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& specs)
		: m_specification{ specs }, m_renderer_id{ 0 }, m_depth_attachment{ 0 }
	{
		for (auto spec : m_specification.Attachments.Attachments)
		{
			if (!Utilities::IsDepthFormat(spec.Texture_format))
				m_color_attachment_specs.emplace_back(spec);
			else
			{
				if (m_depth_attachment_spec.Texture_format != FramebufferTextureFormat::None)
					KB_CORE_ASSERT(false, "Only one depth buffer attachment is currently supported!");

				m_depth_attachment_spec = spec;
			}
		}

		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		DeleteBuffer();
	}
	void OpenGLFramebuffer::Invalidate()
	{
		if (m_renderer_id)
		{
			DeleteBuffer();

			m_color_attachments.clear();
			m_depth_attachment = 0;
		}
		glCreateFramebuffers(1, &m_renderer_id);
		glBindFramebuffer(GL_FRAMEBUFFER, m_renderer_id);

		bool multisample = m_specification.Samples > 1;

		if (m_color_attachment_specs.size())
		{
			// Avoid unecessary allocations during invalidate
			m_color_attachments.resize(m_color_attachment_specs.size());
			Utilities::CreateTextures(multisample, m_color_attachments.data(), m_color_attachments.size());

			// Color Attachments
			for (size_t i = 0; i < m_color_attachments.size(); ++i)
			{
				Utilities::BindTexture(multisample, m_color_attachments[i]);
				switch (m_color_attachment_specs[i].Texture_format)
				{
				case FramebufferTextureFormat::RGBA8:
					Utilities::AttachColorTexture(m_color_attachments[i], m_specification.Samples, GL_RGBA8, GL_RGBA, m_specification.Width, m_specification.Height, i);
					break;
				case FramebufferTextureFormat::RED_INTEGER:
					Utilities::AttachColorTexture(m_color_attachments[i], m_specification.Samples, GL_R32I, GL_RED_INTEGER, m_specification.Width, m_specification.Height, i);
					break;
				default:
#if KB_DEBUG
					KB_CORE_ASSERT(false, "Unsupported color texture format passed to frame buffer specification!");
#else
					KB_CORE_ERROR("Unsupported color texture format passed to frame buffer specification!");
#endif
					break;
				}
			}

		}

		// Depth Attachment
		if (m_depth_attachment_spec.Texture_format != FramebufferTextureFormat::None)
		{
			Utilities::CreateTextures(multisample, &m_depth_attachment, 1);
			Utilities::BindTexture(multisample, m_depth_attachment);
			switch (m_depth_attachment_spec.Texture_format)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:
				Utilities::AttachDepthTexture(m_depth_attachment, m_specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, 
					m_specification.Width, m_specification.Height);
				break;
			default:
#if KB_DEBUG
				KB_CORE_ASSERT(false, "Unsupported depth texture format passed to frame buffer specification!");
#else
				KB_CORE_ERROR("Unsupported depth texture format passed to frame buffer specification!");
#endif
				break;
			}
		}

		if (m_color_attachments.size() > 1)
		{
			KB_CORE_ASSERT(m_color_attachments.size() <= 4, "Only four concurrent color attachments are currently supported!");
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(m_color_attachments.size(), buffers);
		}
		else if (m_color_attachments.empty())
		{
			// Only depth pass
			glDrawBuffer(GL_NONE);
		}

		KB_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer incomplete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_renderer_id);
		glViewport(0, 0, m_specification.Width, m_specification.Height);

		
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
		m_specification.Width = width;
		m_specification.Height = height;

		Invalidate();
	}

	int OpenGLFramebuffer::ReadPixel(uint32_t attachment_index, int x, int y)
	{
		KB_CORE_ASSERT(attachment_index < m_color_attachments.size(), "index out of bounds error!");
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment_index);
		int pixel_data;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixel_data);
		return pixel_data;
	}

	void OpenGLFramebuffer::ClearAttachment(uint32_t attachment_index, int value)
	{
		KB_CORE_ASSERT(attachment_index < m_color_attachments.size(), "index out of bounds!");

		auto& spec = m_color_attachment_specs[attachment_index];

		glClearTexImage(m_color_attachments[attachment_index], 0,
			Utilities::KablunkTextureFormatToGLenum(spec.Texture_format), GL_INT, &value);
	}

	void OpenGLFramebuffer::DeleteBuffer()
	{
		glDeleteFramebuffers(1, &m_renderer_id);
		glDeleteTextures(m_color_attachments.size(), m_color_attachments.data());
		glDeleteTextures(1, &m_depth_attachment);
	}

}
