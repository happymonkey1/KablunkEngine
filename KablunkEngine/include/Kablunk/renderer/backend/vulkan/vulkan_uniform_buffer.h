#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_UNIFORM_BUFFER_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_UNIFORM_BUFFER_H

#include "Kablunk/renderer/backend/uniform_buffer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_allocator.h"

#include <vulkan/vulkan.h>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk
class vulkan_uniform_buffer final : public uniform_buffer
{
public:
	vulkan_uniform_buffer(u32 p_size);
	~vulkan_uniform_buffer() override;

	void set_data(const void* p_data, u32 p_size, u32 p_offset = 0) override;
	void rt_set_data(const void* p_data, u32 p_size, u32 p_offset = 0) override;

	const VkDescriptorBufferInfo& get_vk_descriptor_buffer_info() const { return m_descriptor_info; }
private:
	void rt_invalidate();
	void release();
private:
	VkBuffer m_buffer;
	VkDescriptorBufferInfo m_descriptor_info{};
	VmaAllocation m_vk_allocation = nullptr;

    u32 m_size = 0;
	std::string m_name;
	VkShaderStageFlagBits m_shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;

	uint8_t* m_local_storage;
};
} // end namespace kb::render::backend::vk

#endif
