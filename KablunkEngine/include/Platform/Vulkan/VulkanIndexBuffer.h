#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_INDEX_BUFFER_H
#define KABLUNK_PLATFORM_VULKAN_INDEX_BUFFER_H

#include "Kablunk/Core/owning_buffer.h"
#include "Kablunk/Renderer/Buffer.h"

#include "Platform/Vulkan/VulkanAllocator.h"

#include <vulkan/vulkan.h>

namespace kb
{ // start namespace kb

class VulkanIndexBuffer final : public IndexBuffer
{
public:
	VulkanIndexBuffer(uint32_t size);
	VulkanIndexBuffer(const void* data, uint32_t size = 0);
	virtual ~VulkanIndexBuffer() override;

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

} // end namespace kb

#endif
