#pragma once
#ifndef KABLUNK_RENDERER_UNIFORM_BUFFER_SET_H
#define KABLUNK_RENDERER_UNIFORM_BUFFER_SET_H

#include "Kablunk/Core/Core.h"

#include "Kablunk/Renderer/UniformBuffer.h"

namespace Kablunk
{

	class UniformBufferSet : public RefCounted
	{
	public:
		virtual ~UniformBufferSet() = default;

		virtual void Create(uint32_t size, uint32_t binding) = 0;

		virtual IntrusiveRef<UniformBuffer> Get(uint32_t binding, uint32_t set = 0, uint32_t frame = 0) = 0;
		virtual void Set(IntrusiveRef<UniformBuffer> uniform_buffer, uint32_t set = 0, uint32_t frame = 0) = 0;

		static IntrusiveRef<UniformBufferSet> Create(uint32_t frames);
	};

}

#endif
