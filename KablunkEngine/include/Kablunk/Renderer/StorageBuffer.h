#pragma once
#ifndef KABLUNK_RENDERER_STORAGE_BUFFER_H
#define KABLUNK_RENDERER_STORAGE_BUFFER_H

#include "Kablunk/Core/Core.h"

namespace Kablunk
{

	class StorageBuffer : public RefCounted
	{
	public:
		virtual ~StorageBuffer() = default;

		virtual void SetData(const void* data, size_t size, uint32_t offset = 0) = 0;
		virtual void RT_SetData(const void* data, size_t size, uint32_t offset = 0) = 0;
		virtual void Resize(size_t new_size) = 0;

		virtual uint32_t GetBinding() = 0;

		static ref<StorageBuffer> Create(size_t size, uint32_t binding);
	};

}

#endif
