#pragma once
#ifndef KABLUNK_RENDERER_STORAGE_BUFFER_SET_H
#define KABLUNK_RENDERER_STORAGE_BUFFER_SET_H

#include "Kablunk/Core/Core.h"

#include "Kablunk/Renderer/StorageBuffer.h"

namespace Kablunk
{
	
	class StorageBufferSet : public RefCounted
	{
	public:
		virtual ~StorageBufferSet() = default;

		virtual void Create(uint32_t size, uint32_t binding) = 0;

		virtual ref<StorageBuffer> Get(uint32_t binding, uint32_t set, uint32_t frame) = 0;
		virtual void Set(ref<StorageBuffer> storage_buffer, uint32_t set = 0, uint32_t frame = 0) = 0;
		virtual void Resize(uint32_t binding, uint32_t set, uint32_t new_size) = 0;

		static ref<StorageBufferSet> Create(uint32_t frames);
	};

}

#endif
