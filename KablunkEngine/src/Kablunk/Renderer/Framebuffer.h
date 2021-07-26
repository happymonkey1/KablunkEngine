#ifndef KABLUNK_RENDERER_FRAMEBUFFER_H
#define KABLUNK_RENDERER_FRAMEBUFFER_H

#include "Kablunk/Core/Core.h"

namespace Kablunk
{
	enum class FramebufferTextureFormat
	{
		None = 0,

		//Color
		RGBA8,
		RED_INTEGER,

		// Depth / Stencil
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format)
			: Texture_format{ format } { }

		FramebufferTextureFormat Texture_format{ FramebufferTextureFormat::None };
		// #TODO filtering and wrap
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification>& attachments)
			: Attachments{ attachments } {}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	// Properties struct
	struct FramebufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		FramebufferAttachmentSpecification Attachments;
		uint32_t Samples = 1;

		bool Swap_chain_target = false; //glBindFrameBuffer(0);
	};

	class Framebuffer
	{
	public:

		virtual ~Framebuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual int ReadPixel(uint32_t attachment_index, int x, int y) = 0;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
		virtual const FramebufferSpecification& GetSpecification() const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& specs);
	};
}

#endif
