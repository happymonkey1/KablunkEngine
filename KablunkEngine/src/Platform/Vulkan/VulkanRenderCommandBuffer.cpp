#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanRenderCommandBuffer.h"
#include "Platform/Vulkan/VulkanContext.h"

#include "Kablunk/Renderer/Renderer.h"

namespace Kablunk
{

	VulkanRenderCommandBuffer::VulkanRenderCommandBuffer(uint32_t count /*= 0*/, const std::string& debug_name /*= ""*/)
		: m_debug_name{ debug_name }, m_owned_by_swapchain{ false }
	{
		auto device = VulkanContext::Get()->GetDevice();
		uint32_t frames_in_flight = render::get_frames_in_flights();

		VkCommandPoolCreateInfo cmd_pool_create_info{};
		cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmd_pool_create_info.queueFamilyIndex = device->GetPhysicalDevice()->GetQueueFamilyIndices().Graphics_family.value();
		cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		if (vkCreateCommandPool(device->GetVkDevice(), &cmd_pool_create_info, nullptr, &m_command_pool) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create command pool!");

		VkCommandBufferAllocateInfo cmd_buffer_allocation_info{};
		cmd_buffer_allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_buffer_allocation_info.commandPool = m_command_pool;
		cmd_buffer_allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		
		if (count == 0)
			count = frames_in_flight;

		cmd_buffer_allocation_info.commandBufferCount = count;
		m_command_buffers.resize(count);
		if (vkAllocateCommandBuffers(device->GetVkDevice(), &cmd_buffer_allocation_info, m_command_buffers.data()) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to allocate command buffers");

		VkFenceCreateInfo fence_create_info{};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		m_wait_fences.resize(frames_in_flight);
		for (auto& fence : m_wait_fences)
			if (vkCreateFence(device->GetVkDevice(), &fence_create_info, nullptr, &fence) != VK_SUCCESS)
				KB_CORE_ASSERT(false, "Vulkan failed to create fence!");

		// Timestamp queries
		constexpr const uint32_t k_max_user_queries = 10;
		m_timestamp_query_count = 2 + 2 * k_max_user_queries;

		VkQueryPoolCreateInfo query_pool_create_info = {};
		query_pool_create_info.pNext = nullptr;
		query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
		query_pool_create_info.queryCount = m_timestamp_query_count;
		m_timestamp_query_pools.resize(frames_in_flight);
		for (auto& timestamp_query_pool : m_timestamp_query_pools)
			if (vkCreateQueryPool(device->GetVkDevice(), &query_pool_create_info, nullptr, &timestamp_query_pool) != VK_SUCCESS)
				KB_CORE_ASSERT(false, "Vulkan failed to create query pool!");

		m_timestamp_query_results.resize(frames_in_flight);
		for (auto& timestamp_query_result : m_timestamp_query_results)
			timestamp_query_result.resize(m_timestamp_query_count);

		m_execution_gpu_times.resize(frames_in_flight);
		for (auto& execution_gpu_time : m_execution_gpu_times)
			execution_gpu_time.resize(m_timestamp_query_count / 2);

		// #TODO Pipeline statistics queries
	}

	VulkanRenderCommandBuffer::VulkanRenderCommandBuffer(const std::string& debug_name, bool swap_chain)
		: m_debug_name{ debug_name }, m_owned_by_swapchain{ true }
	{
		auto device = VulkanContext::Get()->GetDevice();
		uint32_t frames_in_flight = render::get_frames_in_flights();

		m_command_buffers.resize(frames_in_flight);
		VulkanSwapChain& swapChain = VulkanContext::Get()->GetSwapchain();
		for (uint32_t frame = 0; frame < frames_in_flight; ++frame)
			m_command_buffers[frame] = swapChain.GetDrawCommandBuffer(frame);

		VkQueryPoolCreateInfo query_pool_create_info = {};
		query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		query_pool_create_info.pNext = nullptr;

		// Timestamp queries
        constexpr const uint32_t k_max_user_queries = 10;
		m_timestamp_query_count = 2 + 2 * k_max_user_queries;

		query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
		query_pool_create_info.queryCount = m_timestamp_query_count;
		m_timestamp_query_pools.resize(frames_in_flight);
		for (auto& timestamp_query_pool : m_timestamp_query_pools)
			if (vkCreateQueryPool(device->GetVkDevice(), &query_pool_create_info, nullptr, &timestamp_query_pool) != VK_SUCCESS)
				KB_CORE_ASSERT(false, "Vulkan failed to create query pool!");

		m_timestamp_query_results.resize(frames_in_flight);
		for (auto& timestamp_query_result : m_timestamp_query_results)
			timestamp_query_result.resize(m_timestamp_query_count);

		m_execution_gpu_times.resize(frames_in_flight);
		for (auto& execution_gpu_time : m_execution_gpu_times)
			execution_gpu_time.resize(m_timestamp_query_count / 2);

		// #TODO pipeline statistics
	}

	VulkanRenderCommandBuffer::~VulkanRenderCommandBuffer()
	{
		if (m_owned_by_swapchain)
			return;

		VkCommandPool command_pool = m_command_pool;
		render::submit_resource_free([command_pool]()
			{
				auto device = VulkanContext::Get()->GetDevice();
				vkDestroyCommandPool(device->GetVkDevice(), command_pool, nullptr);
			});
	}

	void VulkanRenderCommandBuffer::Begin()
	{
		m_timestamp_next_available_query = 2;

		IntrusiveRef<VulkanRenderCommandBuffer> instance = this;
		render::submit([instance]() mutable
			{
				uint32_t frame_index = render::rt_get_current_frame_index();

				VkCommandBufferBeginInfo cmd_buf_info = {};
				cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				cmd_buf_info.pNext = nullptr;

				VkCommandBuffer vk_command_buffer = nullptr;
				if (instance->m_owned_by_swapchain)
				{
					VulkanSwapChain& swap_chain = VulkanContext::Get()->GetSwapchain();
					vk_command_buffer = swap_chain.GetDrawCommandBuffer(frame_index);
				}
				else
					vk_command_buffer = instance->m_command_buffers[frame_index];
				
				if (vkBeginCommandBuffer(vk_command_buffer, &cmd_buf_info) != VK_SUCCESS)
					KB_CORE_ASSERT(false, "Vulkan failed to begin command buffer");

				// Timestamp query
				vkCmdResetQueryPool(vk_command_buffer, instance->m_timestamp_query_pools[frame_index], 0, instance->m_timestamp_query_count);
				vkCmdWriteTimestamp(vk_command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, instance->m_timestamp_query_pools[frame_index], 0);

				// #TODO Pipeline stats query
			});

	}

	void VulkanRenderCommandBuffer::End()
	{
		IntrusiveRef<VulkanRenderCommandBuffer> instance = this;
		render::submit([instance]()
			{
				uint32_t frame_index = render::rt_get_current_frame_index();
				VkCommandBuffer command_buffer = instance->m_command_buffers[frame_index];

				vkCmdWriteTimestamp(command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, instance->m_timestamp_query_pools[frame_index], 1);

				if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
					KB_CORE_ASSERT(false, "Vulkan failed to end command buffer");
			});

	}

	void VulkanRenderCommandBuffer::Submit()
	{
		if (m_owned_by_swapchain)
			return;

		IntrusiveRef<VulkanRenderCommandBuffer> instance = this;
		render::submit([instance]() mutable
			{
				auto device = VulkanContext::Get()->GetDevice();
				VkDevice vk_device = device->GetVkDevice();

				uint32_t frame_index = render::rt_get_current_frame_index();

				VkSubmitInfo submit_info{};
				submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submit_info.commandBufferCount = 1;
				VkCommandBuffer command_buffer = instance->m_command_buffers[frame_index];
				submit_info.pCommandBuffers = &command_buffer;

				if (vkWaitForFences(vk_device, 1, &instance->m_wait_fences[frame_index], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
					KB_CORE_ASSERT(false, "Vulkan failed to wait for fences!");
					
				if (vkResetFences(vk_device, 1, &instance->m_wait_fences[frame_index]) != VK_SUCCESS)
					KB_CORE_ASSERT(false, "Vulkan failed to reset fences!");

				if(vkQueueSubmit(device->GetGraphicsQueue(), 1, &submit_info, instance->m_wait_fences[frame_index]) != VK_SUCCESS)
					KB_CORE_ASSERT(false, "Vulkan failed to submit queue")

				// retrieve timestamp query results
				vkGetQueryPoolResults(vk_device, instance->m_timestamp_query_pools[frame_index], 0, instance->m_timestamp_next_available_query,
					instance->m_timestamp_next_available_query * sizeof(uint64_t), instance->m_timestamp_query_results[frame_index].data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);

				for (uint32_t i = 0; i < instance->m_timestamp_next_available_query; i += 2)
				{
					uint64_t startTime = instance->m_timestamp_query_results[frame_index][i];
					uint64_t endTime = instance->m_timestamp_query_results[frame_index][i + 1];
					float ns_time = endTime > startTime ? (endTime - startTime) * device->GetPhysicalDevice()->GetLimits().timestampPeriod : 0.0f;
					instance->m_execution_gpu_times[frame_index][i / 2] = ns_time * 0.000001f; // time in ms
				}

				// #TODO pipeline stats results
			});

	}

	uint64_t VulkanRenderCommandBuffer::BeginTimestampQuery()
	{
		uint64_t query_index = m_timestamp_next_available_query;
		m_timestamp_next_available_query += 2;
		IntrusiveRef<VulkanRenderCommandBuffer> instance = this;
		render::submit([instance, query_index]()
			{
				uint32_t frame_index = render::rt_get_current_frame_index();
				VkCommandBuffer command_buffer = instance->m_command_buffers[frame_index];
				vkCmdWriteTimestamp(command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, instance->m_timestamp_query_pools[frame_index], static_cast<uint32_t>(query_index));
			});

		return query_index;
	}

	void VulkanRenderCommandBuffer::EndTimestampQuery(uint64_t query_index)
	{
		IntrusiveRef<VulkanRenderCommandBuffer> instance = this;
		render::submit([instance, query_index]()
			{
				uint32_t frame_index = render::rt_get_current_frame_index();
				VkCommandBuffer command_buffer = instance->m_command_buffers[frame_index];
				vkCmdWriteTimestamp(command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, instance->m_timestamp_query_pools[frame_index], static_cast<uint32_t>(query_index + 1));
			});
	}

}
