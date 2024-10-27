#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_RENDER_COMMAND_BUFFER_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_RENDER_COMMAND_BUFFER_H

#include "Kablunk/renderer/backend/render_command_buffer.h"

#include "Kablunk/Core/Core.h"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk
class vulkan_render_command_buffer final : public render_command_buffer
{
public:
	vulkan_render_command_buffer(uint32_t count = 0, const std::string& debug_name = "");
	vulkan_render_command_buffer(const std::string& debug_name, bool swap_chain);
	~vulkan_render_command_buffer() override;

	virtual void begin() override;
	virtual void end() override;
	virtual void submit() override;

	virtual float get_execution_gpu_time(uint32_t frame_index, uint32_t query_index /* = 0 */) const override
	{
		if (query_index / 2 >= m_timestamp_next_available_query / 2)
			return 0.0f;

		return m_execution_gpu_times[frame_index][query_index / 2];
	}

	virtual uint64_t begin_timestamp_query() override;
	virtual void end_timestamp_query(uint64_t query_index) override;

	inline VkCommandBuffer GetCommandBuffer(uint32_t frame_index) const
	{
		KB_CORE_ASSERT(frame_index < m_command_buffers.size(), "index out of range!");
		return m_command_buffers[frame_index];
	}

    inline VkCommandBuffer get_active_command_buffer() const { return m_active_command_buffer; }
private:
	std::string m_debug_name;

	VkCommandPool m_command_pool = nullptr;
	std::vector<VkCommandBuffer> m_command_buffers;
    VkCommandBuffer m_active_command_buffer = nullptr;
	std::vector<VkFence> m_wait_fences;

	bool m_owned_by_swapchain = false;

	uint32_t m_timestamp_query_count = 0;
	uint32_t m_timestamp_next_available_query = 2;

	std::vector<VkQueryPool> m_timestamp_query_pools;
	std::vector<std::vector<uint64_t>> m_timestamp_query_results;
	std::vector<std::vector<float>> m_execution_gpu_times;
};
} // end namespace kb::render::backend::vk

#endif
