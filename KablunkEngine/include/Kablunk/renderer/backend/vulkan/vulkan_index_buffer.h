#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_INDEX_BUFFER_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_INDEX_BUFFER_H

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/Core/owning_buffer.h"

#include "kablunk/renderer/backend/vulkan/vulkan_allocator.h"
#include "Kablunk/renderer/backend/buffer.h"

#include <vulkan/vulkan.h>


namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

class vulkan_index_buffer final : public index_buffer
{
public:
	vulkan_index_buffer(u32 size);
	vulkan_index_buffer(const void* data, u32 size = 0);
	~vulkan_index_buffer() override;

	void bind() const override;
	void unbind() const override;

	void set_data(const void* buffer, u32 size, u32 offset = 0) override;

    u32 get_count() const noexcept override { return m_size / sizeof(u32); };
	u32 get_size() const noexcept override { return m_size; }

	VkBuffer GetVkBuffer() const noexcept { return m_vk_buffer; }
private:
	VkBuffer m_vk_buffer = nullptr;
	owning_buffer m_local_data;
    u32 m_size = 0;
	VmaAllocation m_vk_allocation = nullptr;
};

} // end namespace kb::render::backend::vk

#endif
