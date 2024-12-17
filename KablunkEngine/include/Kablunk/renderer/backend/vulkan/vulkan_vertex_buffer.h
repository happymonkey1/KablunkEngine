#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_VERTEX_BUFFER_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_VERTEX_BUFFER_H

#include "Kablunk/Core/owning_buffer.h"
#include "Kablunk/Core/Core.h"

#include "Kablunk/renderer/backend/buffer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_allocator.h"

#include <vulkan/vulkan.h>


namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

class vulkan_vertex_buffer final : public vertex_buffer
{
public:
	vulkan_vertex_buffer(
        weak_ptr<vulkan_logical_device> p_device,
        const void* data,
        uint32_t size,
        vertex_buffer_usage_t usage = vertex_buffer_usage_t::Static
    );

	vulkan_vertex_buffer(
        weak_ptr<vulkan_logical_device> p_device,
        uint32_t size,
        vertex_buffer_usage_t usage = vertex_buffer_usage_t::Dynamic
    );
	~vulkan_vertex_buffer() override;

	void bind() const override;

    auto rt_vk_bind_buffer(
        const VkCommandBuffer p_vk_command_buffer,
        const u32 p_binding
    ) const noexcept -> void
    {
        constexpr VkDeviceSize k_offsets[1] = { 0 };
        vkCmdBindVertexBuffers(
            p_vk_command_buffer,
            p_binding,
            1,
            &m_vk_buffer,
            k_offsets
        );
    }

	void unbind() const override;

	void set_data(const void* data, uint32_t size, uint32_t offset = 0) override;
	void rt_set_data(const void* data, uint32_t size, uint32_t offset = 0) override;

	VkBuffer get_vk_buffer() const { return m_vk_buffer; }

	void set_layout(const buffer_layout& layout) override;
	const buffer_layout& get_layout() const override;
private:
	uint32_t m_size = 0;
	owning_buffer m_local_data{};
    weak_ptr<vulkan_logical_device> m_device = nullptr;

	VkBuffer m_vk_buffer = nullptr;
	VmaAllocation m_memory_allocation{};
};

} // end namespace kb::render::backend::vk

#endif
