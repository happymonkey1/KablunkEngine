#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_VERTEX_BUFFER_H
#define KABLUNK_PLATFORM_VULKAN_VERTEX_BUFFER_H

#include "Kablunk/Core/Buffer.h"
#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Buffer.h"

#include "Platform/Vulkan/VulkanAllocator.h"

#include <vulkan/vulkan.h>

namespace Kablunk
{

	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(const void* data, uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Static);
		VulkanVertexBuffer(uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Dynamic);
		virtual ~VulkanVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
		virtual void RT_SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

		virtual void SetLayout(const BufferLayout& layout) override;
		virtual const BufferLayout& GetLayout() const override;

		virtual RendererID GetRendererID() const override;
	private:
		uint32_t m_size = 0;
		Buffer m_local_data;

		VkBuffer m_vk_buffer = nullptr;
		VmaAllocation m_memory_allocation;
	};

}

#endif
