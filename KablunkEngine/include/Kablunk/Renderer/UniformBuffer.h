#ifndef KABLUNK_RENDERER_UNIFORMBUFFER_H
#define KABLUNK_RENDERER_UNIFORMBUFFER_H

#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Renderer/Buffer.h"

namespace Kablunk
{
	class UniformBuffer : public RefCounted
	{
	public:
		virtual ~UniformBuffer() { };
		virtual void SetData(const void* data, uint32_t size, uint32_t offest = 0) = 0;

		virtual RendererID GetBinding() const = 0;

		static IntrusiveRef<UniformBuffer> Create(uint32_t size, uint32_t binding);
	};
}
#endif
