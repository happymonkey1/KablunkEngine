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
	vulkan_vertex_buffer(const void* data, uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Static);
	vulkan_vertex_buffer(uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Dynamic);
	virtual ~vulkan_vertex_buffer() override;

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
	virtual void RT_SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

	VkBuffer GetVkBuffer() const { return m_vk_buffer; }

	virtual void SetLayout(const BufferLayout& layout) override;
	virtual const BufferLayout& GetLayout() const override;

	virtual RendererID GetRendererID() const override;
private:
	uint32_t m_size = 0;
	owning_buffer m_local_data;

	VkBuffer m_vk_buffer = nullptr;
	VmaAllocation m_memory_allocation;
};

} // end namespace kb::render::backend::vk

#endif
