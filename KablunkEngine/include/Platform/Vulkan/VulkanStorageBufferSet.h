#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_STORAGE_BUFFER_SET_H
#define KABLUNK_PLATFORM_VULKAN_STORAGE_BUFFER_SET_H

#include "Kablunk/Renderer/StorageBufferSet.h"

#include <map>

namespace Kablunk
{
	class VulkanStorageBufferSet : public StorageBufferSet
	{
	public:
		explicit VulkanStorageBufferSet(uint32_t frames);
		virtual ~VulkanStorageBufferSet() = default;

		virtual void Create(uint32_t size, uint32_t binding) override;

		virtual IntrusiveRef<StorageBuffer> Get(uint32_t binding, uint32_t set, uint32_t frame) override;
		virtual void Set(IntrusiveRef<StorageBuffer> storage_buffer, uint32_t set = 0, uint32_t frame = 0) override;
		virtual void Resize(uint32_t binding, uint32_t set, uint32_t new_size) override;
	private:
		uint32_t m_frames;
		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, IntrusiveRef<StorageBuffer>>>> m_storage_buffers;
	};
}

#endif
