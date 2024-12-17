#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_RENDER_COMMAND_BUFFER_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_RENDER_COMMAND_BUFFER_H

#include "Kablunk/renderer/backend/render_command_buffer.h"
#include "Kablunk/renderer/backend/vulkan/vulkan_logical_device.h"
#include "Kablunk/renderer/backend/vulkan/vulkan_swap_chain.h"

#include "Kablunk/Core/Core.h"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>



namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk
class vulkan_render_command_buffer final : public render_command_buffer
{
public:
	vulkan_render_command_buffer(
        VkDevice p_vk_device,
        u32 p_graphics_queue_family_index,
        u32 p_count,
        std::string p_debug_name
    );
	vulkan_render_command_buffer(
        VkDevice p_vk_device,
        weak_ptr<vulkan_swap_chain> p_swap_chain,
        std::string p_debug_name
    );
	~vulkan_render_command_buffer() override;

	void begin() override;
	void end() override;
	void submit() override;

	float get_execution_gpu_time(u32 frame_index, u32 query_index /* = 0 */) const override
	{
		if (query_index / 2 >= m_timestamp_next_available_query / 2)
			return 0.0f;

		return m_execution_gpu_times[frame_index][query_index / 2];
	}

    u64 begin_timestamp_query() override;
	void end_timestamp_query(u64 query_index) override;

	VkCommandBuffer get_vk_command_buffer(u32 frame_index) const
	{
		KB_CORE_ASSERT(frame_index < m_command_buffers.size(), "index out of range!");
		return m_command_buffers[frame_index];
	}

    VkCommandBuffer get_active_command_buffer() const { return m_active_command_buffer; }

private:
	std::string m_debug_name;
    VkDevice m_vk_device = nullptr;

	VkCommandPool m_command_pool = nullptr;
	std::vector<VkCommandBuffer> m_command_buffers;
    VkCommandBuffer m_active_command_buffer = nullptr;
	std::vector<VkFence> m_wait_fences;

	bool m_owned_by_swapchain = false;

    u32 m_timestamp_query_count = 0;
    u32 m_timestamp_next_available_query = 2;

	std::vector<VkQueryPool> m_timestamp_query_pools;
	std::vector<std::vector<u64>> m_timestamp_query_results;
	std::vector<std::vector<f32>> m_execution_gpu_times;
};
} // end namespace kb::render::backend::vk

#endif
