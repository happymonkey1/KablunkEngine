#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_INDEX_BUFFER_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_INDEX_BUFFER_H

#include "Kablunk/Core/owning_buffer.h"

#include "kablunk/renderer/backend/vulkan/vulkan_allocator.h"
#include "Kablunk/renderer/backend/buffer.h"

#include <vulkan/vulkan.h>


namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

class vulkan_index_buffer final : public IndexBuffer
{
public:
	vulkan_index_buffer(uint32_t size);
	vulkan_index_buffer(const void* data, uint32_t size = 0);
	virtual ~vulkan_index_buffer() override;

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual void SetData(const void* buffer, uint32_t size, uint32_t offset = 0);

	virtual const uint32_t GetCount() const { return m_size / sizeof(uint32_t); };
	virtual uint32_t GetSize() const { return m_size; }

	virtual RendererID GetRendererID() const override;

	VkBuffer GetVkBuffer() { return m_vk_buffer; }
private:
	VkBuffer m_vk_buffer = nullptr;
	owning_buffer m_local_data;
	uint32_t m_size = 0;

	VmaAllocation m_vk_allocation = nullptr;
};

} // end namespace kb::render::backend::vk

#endif
