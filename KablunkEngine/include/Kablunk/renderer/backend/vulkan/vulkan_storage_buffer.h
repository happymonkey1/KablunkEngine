#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_STORAGE_BUFFER_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_STORAGE_BUFFER_H

#include "Kablunk/renderer/backend/storage_buffer.h"

#include "kablunk/renderer/backend/vulkan/vulkan_allocator.h"

#include <vulkan/vulkan.h>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk
class vulkan_storage_buffer final : public storage_buffer
{
public:
	vulkan_storage_buffer(size_t size, uint32_t binding);
	virtual ~vulkan_storage_buffer() override;

	virtual void set_data(const void* data, size_t size, uint32_t offset = 0) override;
	virtual void rt_set_data(const void* data, size_t size, uint32_t offset = 0) override;
	virtual void resize(size_t new_size) override;

	virtual uint32_t get_binding() { return m_binding; };

	const VkDescriptorBufferInfo& GetVkDescriptorInfo() const { return m_vk_descriptor_info; }
private:
	void Release();
	void RT_Invalidate();
private:
	VmaAllocation m_vk_memory_allocation;
	VkBuffer m_vk_buffer{};

	VkDescriptorBufferInfo m_vk_descriptor_info{};

	size_t m_size;
	uint32_t m_binding;

	std::string m_name;
	VkShaderStageFlagBits m_vk_shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;

	uint8_t* m_local_storage = nullptr; // #TODO change to buffer;
};
} // end namespace kb::render::backend::vk

#endif
