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

    auto rt_vk_bind_buffer(const VkCommandBuffer p_vk_command_buffer) const noexcept -> void
    {
        constexpr VkDeviceSize k_offset = 0;
        vkCmdBindIndexBuffer(
            p_vk_command_buffer,
            m_vk_buffer,
            k_offset,
            get_vk_index_type()
        );
    }

	void unbind() const override;

	void set_data(const void* buffer, u32 size, u32 offset = 0) override;

    u32 get_count() const noexcept override { return m_size / sizeof(u32); };
	u32 get_size() const noexcept override { return m_size; }

    auto get_index_type() const noexcept -> index_type_t { return m_index_type; }
    auto get_vk_index_type() const noexcept -> VkIndexType;

	VkBuffer get_vk_buffer() const noexcept { return m_vk_buffer; }
private:
	VkBuffer m_vk_buffer = nullptr;
	owning_buffer m_local_data;
    u32 m_size = 0;
	VmaAllocation m_vk_allocation = nullptr;
    index_type_t m_index_type = index_type_t::u32;
};

} // end namespace kb::render::backend::vk

#endif
