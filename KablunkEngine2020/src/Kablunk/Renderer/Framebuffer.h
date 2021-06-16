#ifndef KABLUNK_RENDERER_FRAMEBUFFER_H
#define KABLUNK_RENDERER_FRAMEBUFFER_H

#include "Kablunk/Core/Core.h"

namespace Kablunk
{
	// Properties struct
	struct FrameBufferSpecification
	{
		uint32_t width, height;
		uint32_t samples = 1;

		bool swap_chain_target = false; //glBindFrameBuffer(0);
	};

	class Framebuffer
	{
	public:

		virtual ~Framebuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetColorAttachmentRendererID() const = 0;
		virtual const FrameBufferSpecification& GetSpecification() const = 0;

		static Ref<Framebuffer> Create(const FrameBufferSpecification& specs);
	};
}

#endif