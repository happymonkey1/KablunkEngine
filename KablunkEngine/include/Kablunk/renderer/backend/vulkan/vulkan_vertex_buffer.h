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

class vulkan_vertex_buffer final : public VertexBuffer
{
public:
	vulkan_vertex_buffer(
        weak_arc<vulkan_logical_device> p_device,
        const void* data,
        uint32_t size,
        VertexBufferUsage usage = VertexBufferUsage::Static
    );

	vulkan_vertex_buffer(
        weak_arc<vulkan_logical_device> p_device,
        uint32_t size,
        VertexBufferUsage usage = VertexBufferUsage::Dynamic
    );
	~vulkan_vertex_buffer() override;

	void Bind() const override;
	void Unbind() const override;

	void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
	void RT_SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

	VkBuffer GetVkBuffer() const { return m_vk_buffer; }

	void SetLayout(const BufferLayout& layout) override;
	const BufferLayout& GetLayout() const override;

	RendererID GetRendererID() const override;
private:
	uint32_t m_size = 0;
	owning_buffer m_local_data{};
    weak_arc<vulkan_logical_device> m_device = nullptr;

	VkBuffer m_vk_buffer = nullptr;
	VmaAllocation m_memory_allocation{};
};

} // end namespace kb::render::backend::vk

#endif
