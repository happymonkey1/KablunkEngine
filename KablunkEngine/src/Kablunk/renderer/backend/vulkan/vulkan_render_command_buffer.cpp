#include "kablunkpch.h"

#include "kablunk/renderer/backend/vulkan/vulkan_render_command_buffer.h"

#include "Kablunk/renderer/renderer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_context.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

vulkan_render_command_buffer::vulkan_render_command_buffer(uint32_t count /*= 0*/, const std::string& debug_name /*= ""*/)
	: m_debug_name{ debug_name }
{
	auto device = vulkan_context::get()->get_device();
	uint32_t frames_in_flight = render::get_frames_in_flight();

	VkCommandPoolCreateInfo cmd_pool_create_info{};
	cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_create_info.queueFamilyIndex = device->get_physical_device()->GetQueueFamilyIndices().Graphics_family.value();
	cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (vkCreateCommandPool(device->get_vk_device(), &cmd_pool_create_info, nullptr, &m_command_pool) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "Vulkan failed to create command pool!");

	VkCommandBufferAllocateInfo cmd_buffer_allocation_info{};
	cmd_buffer_allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_buffer_allocation_info.commandPool = m_command_pool;
	cmd_buffer_allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	if (count == 0)
		count = frames_in_flight;

	cmd_buffer_allocation_info.commandBufferCount = count;
	m_command_buffers.resize(count);
	if (vkAllocateCommandBuffers(device->get_vk_device(), &cmd_buffer_allocation_info, m_command_buffers.data()) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "Vulkan failed to allocate command buffers");

	VkFenceCreateInfo fence_create_info{};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	m_wait_fences.resize(frames_in_flight);
	for (auto& fence : m_wait_fences)
		if (vkCreateFence(device->get_vk_device(), &fence_create_info, nullptr, &fence) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create fence!");

	// Timestamp queries
	constexpr uint32_t k_max_user_queries = 10;
	m_timestamp_query_count = 2 + 2 * k_max_user_queries;

	VkQueryPoolCreateInfo query_pool_create_info = {};
	query_pool_create_info.pNext = nullptr;
	query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
	query_pool_create_info.queryCount = m_timestamp_query_count;
	m_timestamp_query_pools.resize(frames_in_flight);
	for (auto& timestamp_query_pool : m_timestamp_query_pools)
		if (vkCreateQueryPool(device->get_vk_device(), &query_pool_create_info, nullptr, &timestamp_query_pool) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create query pool!");

	m_timestamp_query_results.resize(frames_in_flight);
	for (auto& timestamp_query_result : m_timestamp_query_results)
		timestamp_query_result.resize(m_timestamp_query_count);

	m_execution_gpu_times.resize(frames_in_flight);
	for (auto& execution_gpu_time : m_execution_gpu_times)
		execution_gpu_time.resize(m_timestamp_query_count / 2);

	// #TODO Pipeline statistics queries
}

vulkan_render_command_buffer::vulkan_render_command_buffer(const std::string& debug_name, bool swap_chain)
	: m_debug_name{ debug_name }, m_owned_by_swapchain{ true }
{
	auto device = vulkan_context::get()->get_device();
    const uint32_t frames_in_flight = render::get_frames_in_flight();

	m_command_buffers.resize(frames_in_flight);
    for (size_t i = 0; i < frames_in_flight; ++i)
    {
        auto& command_buffer = m_command_buffers.at(i);
        command_buffer = vulkan_context::get()->get_vulkan_swap_chain()->GetDrawCommandBuffer(i);
    }

	VkQueryPoolCreateInfo query_pool_create_info = {};
	query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	query_pool_create_info.pNext = nullptr;

	// Timestamp queries
    constexpr uint32_t k_max_user_queries = 10;
	m_timestamp_query_count = 2 + 2 * k_max_user_queries;

	query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
	query_pool_create_info.queryCount = m_timestamp_query_count;
	m_timestamp_query_pools.resize(frames_in_flight);
	for (auto& timestamp_query_pool : m_timestamp_query_pools)
		if (vkCreateQueryPool(device->get_vk_device(), &query_pool_create_info, nullptr, &timestamp_query_pool) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create query pool!");

	m_timestamp_query_results.resize(frames_in_flight);
	for (auto& timestamp_query_result : m_timestamp_query_results)
		timestamp_query_result.resize(m_timestamp_query_count);

	m_execution_gpu_times.resize(frames_in_flight);
	for (auto& execution_gpu_time : m_execution_gpu_times)
		execution_gpu_time.resize(m_timestamp_query_count / 2);

	// #TODO pipeline statistics
}

vulkan_render_command_buffer::~vulkan_render_command_buffer()
{
	if (m_owned_by_swapchain)
		return;

	VkCommandPool command_pool = m_command_pool;
	render::submit_resource_free([command_pool]()
		{
            KB_CORE_INFO("Destroying command pool {}", static_cast<void*>(command_pool));
			auto device = vulkan_context::get()->get_device();
			vkDestroyCommandPool(device->get_vk_device(), command_pool, nullptr);
		});
}

void vulkan_render_command_buffer::begin()
{
	m_timestamp_next_available_query = 2;

    arc instance{ this };
	render::submit([instance]() mutable
		{
            const uint32_t frame_index = render::rt_get_current_frame_index();

			VkCommandBufferBeginInfo cmd_buf_info = {};
			cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			cmd_buf_info.pNext = nullptr;

			VkCommandBuffer vk_command_buffer;
			if (instance->m_owned_by_swapchain)
			{
				auto& swap_chain = vulkan_context::get()->get_vulkan_swap_chain();
				vk_command_buffer = swap_chain->GetDrawCommandBuffer(frame_index);
                instance->m_command_buffers[frame_index] = vk_command_buffer;
			}
			else
				vk_command_buffer = instance->m_command_buffers[frame_index];

            KB_CORE_ASSERT(vk_command_buffer, "[VulkanRenderCommandBuffer]: vk_command_buffer is null!");
            instance->m_active_command_buffer = vk_command_buffer;

			if (vkBeginCommandBuffer(vk_command_buffer, &cmd_buf_info) != VK_SUCCESS)
				KB_CORE_ASSERT(false, "Vulkan failed to begin command buffer");

			// Timestamp query
			vkCmdResetQueryPool(vk_command_buffer, instance->m_timestamp_query_pools[frame_index], 0, instance->m_timestamp_query_count);
			vkCmdWriteTimestamp(vk_command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, instance->m_timestamp_query_pools[frame_index], 0);

			// #TODO Pipeline stats query
		});
}

void vulkan_render_command_buffer::end()
{
    arc instance{ this };
	render::submit([instance]() mutable
		{
            const uint32_t frame_index = render::rt_get_current_frame_index();
            const VkCommandBuffer command_buffer = instance->m_active_command_buffer;
            KB_CORE_ASSERT(command_buffer, "[VulkanRenderCommandBuffer]: active command buffer is null!");

			vkCmdWriteTimestamp(command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, instance->m_timestamp_query_pools[frame_index], 1);

			if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
				KB_CORE_ASSERT(false, "Vulkan failed to end command buffer");

            instance->m_active_command_buffer = nullptr;
		});

}

void vulkan_render_command_buffer::submit()
{
	if (m_owned_by_swapchain)
		return;

    arc instance{ this };
	render::submit([instance]() mutable
		{
			auto device = vulkan_context::get()->get_device();
            const VkDevice vk_device = device->get_vk_device();

            const uint32_t frame_index = render::rt_get_current_frame_index();

			VkSubmitInfo submit_info{};
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.commandBufferCount = 1;
            const VkCommandBuffer command_buffer = instance->m_command_buffers[frame_index];
			submit_info.pCommandBuffers = &command_buffer;

            if (vkWaitForFences(vk_device, 1, &instance->m_wait_fences[frame_index], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
                KB_CORE_ASSERT(false, "Vulkan failed to wait for fences!");

			if (vkResetFences(vk_device, 1, &instance->m_wait_fences[frame_index]) != VK_SUCCESS)
				KB_CORE_ASSERT(false, "Vulkan failed to reset fences!");

			if(vkQueueSubmit(device->get_vk_graphics_queue(), 1, &submit_info, instance->m_wait_fences[frame_index]) != VK_SUCCESS)
				KB_CORE_ASSERT(false, "Vulkan failed to submit queue");

			// retrieve timestamp query results
			vkGetQueryPoolResults(
                vk_device,
                instance->m_timestamp_query_pools[frame_index],
                0,
                instance->m_timestamp_next_available_query,
				instance->m_timestamp_next_available_query * sizeof(uint64_t),
                instance->m_timestamp_query_results[frame_index].data(),
                sizeof(uint64_t),
                VK_QUERY_RESULT_64_BIT
            );

			for (uint32_t i = 0; i < instance->m_timestamp_next_available_query; i += 2)
			{
				uint64_t startTime = instance->m_timestamp_query_results[frame_index][i];
				uint64_t endTime = instance->m_timestamp_query_results[frame_index][i + 1];
				float ns_time = endTime > startTime ? (endTime - startTime) * device->get_physical_device()->GetLimits().timestampPeriod : 0.0f;
				instance->m_execution_gpu_times[frame_index][i / 2] = ns_time * 0.000001f; // time in ms
			}

			// #TODO pipeline stats results
		});
}

uint64_t vulkan_render_command_buffer::begin_timestamp_query()
{
	uint64_t query_index = m_timestamp_next_available_query;
	m_timestamp_next_available_query += 2;
    arc instance{ this };
	render::submit([instance, query_index]()
		{
			uint32_t frame_index = render::rt_get_current_frame_index();
			VkCommandBuffer command_buffer = instance->m_active_command_buffer;
            KB_CORE_ASSERT(command_buffer, "[VulkanRenderCommandBuffer]: command buffer in BeginTimestampQuery() is null!");
			vkCmdWriteTimestamp(command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, instance->m_timestamp_query_pools[frame_index], static_cast<uint32_t>(query_index));
		});

	return query_index;
}

void vulkan_render_command_buffer::end_timestamp_query(uint64_t query_index)
{
    arc instance{ this };
	render::submit([instance, query_index]()
		{
			uint32_t frame_index = render::rt_get_current_frame_index();
			VkCommandBuffer command_buffer = instance->m_active_command_buffer;
            KB_CORE_ASSERT(command_buffer, "[VulkanRenderCommandBuffer]: command buffer in EndTimestampQuery() is null!");
			vkCmdWriteTimestamp(command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, instance->m_timestamp_query_pools[frame_index], static_cast<uint32_t>(query_index + 1));
		});
}

} // end namespace kb::render::backend::vk
