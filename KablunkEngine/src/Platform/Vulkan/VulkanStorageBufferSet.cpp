#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanStorageBufferSet.h"

namespace kb
{

	VulkanStorageBufferSet::VulkanStorageBufferSet(uint32_t frames)
		: m_frames{ frames }
	{

	}

	void VulkanStorageBufferSet::Create(uint32_t size, uint32_t binding)
	{
		for (uint32_t frame = 0; frame < m_frames; ++frame)
		{
			ref<StorageBuffer> storage_buffer = StorageBuffer::Create(size, binding);
			Set(storage_buffer, 0, frame); // #TODO only one set currently supported!
		}
	}

	ref<StorageBuffer> VulkanStorageBufferSet::Get(uint32_t binding, uint32_t set, uint32_t frame)
	{
		KB_CORE_ASSERT(m_storage_buffers.find(frame) != m_storage_buffers.end(), "frame not found!");
		KB_CORE_ASSERT(m_storage_buffers.at(frame).find(set) != m_storage_buffers.at(frame).end(), "set not found!");
		KB_CORE_ASSERT(m_storage_buffers.at(frame).at(set).find(binding) != m_storage_buffers.at(frame).at(set).end(), "binding not found");

		return m_storage_buffers[frame][set][binding];
	}

	void VulkanStorageBufferSet::Set(ref<StorageBuffer> storage_buffer, uint32_t set /*= 0*/, uint32_t frame /*= 0*/)
	{
		uint32_t binding = storage_buffer->GetBinding();

		m_storage_buffers[frame][set][binding] = storage_buffer;
	}

	void VulkanStorageBufferSet::Resize(uint32_t binding, uint32_t set, uint32_t new_size)
	{
		for (uint32_t frame = 0; frame < m_frames; ++frame)
			m_storage_buffers[frame][set][binding]->Resize(new_size);
	}

}
