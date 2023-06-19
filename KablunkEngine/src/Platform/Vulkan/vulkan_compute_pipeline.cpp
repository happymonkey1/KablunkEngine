#include "kablunkpch.h"

#include "Platform/Vulkan/vulkan_compute_pipeline.h"

#include "Kablunk/Renderer/RenderCommand.h"

namespace kb::vk
{

static VkFence s_vk_compute_fence = nullptr;

compute_pipeline::compute_pipeline(Kablunk::ref<Kablunk::Shader> compute_shader)
    : m_shader{ compute_shader.As<Kablunk::VulkanShader>() }
{
    Kablunk::render::submit(
        [instance = Kablunk::ref{ this }]() mutable
        {
            instance->rt_create_pipeline();
        }
    );

    Kablunk::render::register_shader_dependency(compute_shader, this);
}

void compute_pipeline::execute(VkDescriptorSet* p_descriptor_sets, uint32_t p_descriptor_set_count, uint32_t p_group_count_x, uint32_t p_group_count_y, uint32_t p_group_count_z)
{
    VkDevice vk_device = Kablunk::VulkanContext::Get()->GetDevice()->GetVkDevice();

    VkQueue vk_compute_queue = Kablunk::VulkanContext::Get()->GetDevice()->get_vk_compute_queue();

    VkCommandBuffer vk_compute_command_buffer = Kablunk::VulkanContext::Get()->GetDevice()->GetCommandBuffer(true, true);

    // #TODO set vulkan checkpoint

    // call vulkan api to bind pipeline
    vkCmdBindPipeline(vk_compute_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_vk_compute_pipeline);

    // bind descriptor sets and dispatch compute shader command
    for (uint32_t i = 0; i < p_descriptor_set_count; ++i)
    {
        vkCmdBindDescriptorSets(
            vk_compute_command_buffer, 
            VK_PIPELINE_BIND_POINT_COMPUTE,
            m_vk_compute_pipeline_layout, 
            0, 
            1,
            &p_descriptor_sets[i],
            0, 
            0
        );
        vkCmdDispatch(vk_compute_command_buffer, p_group_count_x, p_group_count_y, p_group_count_z);
    }

    vkEndCommandBuffer(vk_compute_command_buffer);

    if (!s_vk_compute_fence)
    {
        VkFenceCreateInfo vk_fence_create_info{};
        vk_fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        vk_fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        if(vkCreateFence(vk_device, &vk_fence_create_info, nullptr, &s_vk_compute_fence) != VkResult::VK_SUCCESS)
        {
            KB_CORE_ASSERT(false, "[vk::compute_pipeline]: failed to create vk fence!");
        }
    }

    // make sure the previous compute shader in the pipeline has completed
    // #TODO this shouldn't be needed for all cases
    vkWaitForFences(vk_device, 1, &s_vk_compute_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(vk_device, 1, &s_vk_compute_fence);

    VkSubmitInfo vk_compute_submit_info{};
    vk_compute_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vk_compute_submit_info.commandBufferCount = 1;
    vk_compute_submit_info.pCommandBuffers = &vk_compute_command_buffer;
    if(vkQueueSubmit(vk_compute_queue, 1, &vk_compute_submit_info, s_vk_compute_fence) != VkResult::VK_SUCCESS)
    {
        KB_CORE_ASSERT(false, "[vk::compute_pipeline]: failed to submit compute queue!");
    }

    // wait for execution of the shader to complete
    // #TODO this is here for "safety", but blocks our thread...
    vkWaitForFences(vk_device, 1, &s_vk_compute_fence, VK_TRUE, UINT64_MAX);
}

void compute_pipeline::begin(Kablunk::ref<Kablunk::RenderCommandBuffer> p_render_command_buffer /*= nullptr*/)
{
    KB_CORE_ASSERT(!m_vk_active_command_buffer, "[vk::compute_pipeline]: trying to start a new pipeline while there is already an active one?");

    if (p_render_command_buffer)
    {
        uint32_t frameIndex = Kablunk::render::get_current_frame_index();
        m_vk_active_command_buffer = p_render_command_buffer.As<Kablunk::VulkanRenderCommandBuffer>()->GetCommandBuffer(frameIndex);
        m_using_graphics_queue = true;
    }
    else
    {
        m_vk_active_command_buffer = Kablunk::VulkanContext::Get()->GetDevice()->GetCommandBuffer(true, true);
        m_using_graphics_queue = false;
    }
    
    // call vulkan api to bind pipeline
    vkCmdBindPipeline(m_vk_active_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_vk_compute_pipeline);
}

void compute_pipeline::rt_begin(Kablunk::ref<Kablunk::RenderCommandBuffer> p_render_command_buffer /*= nullptr*/)
{
    KB_CORE_ASSERT(!m_vk_active_command_buffer, "[vk::compute_pipeline]: trying to begin a new pipeline while there is already an active one?");

    if (p_render_command_buffer)
    {
        uint32_t frameIndex = Kablunk::render::rt_get_current_frame_index();
        m_vk_active_command_buffer = p_render_command_buffer.As<Kablunk::VulkanRenderCommandBuffer>()->GetCommandBuffer(frameIndex);
        m_using_graphics_queue = true;
    }
    else
    {
        m_vk_active_command_buffer = Kablunk::VulkanContext::Get()->GetDevice()->GetCommandBuffer(true, true);
        m_using_graphics_queue = false;
    }

    // call vulkan api to bind pipeline
    vkCmdBindPipeline(m_vk_active_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_vk_compute_pipeline);
}

void compute_pipeline::end()
{
    KB_CORE_ASSERT(m_vk_active_command_buffer, "[vk::compute_queue]: trying to end a compute pipeline without an active command buffer?");

    VkDevice vk_device = Kablunk::VulkanContext::Get()->GetDevice()->GetVkDevice();
    if (!m_using_graphics_queue)
    {
        VkQueue vk_compute_queue = Kablunk::VulkanContext::Get()->GetDevice()->get_vk_compute_queue();

        vkEndCommandBuffer(m_vk_active_command_buffer);

        if (!s_vk_compute_fence)
        {
            VkFenceCreateInfo vk_fence_creat_info{};
            vk_fence_creat_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            vk_fence_creat_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            if(vkCreateFence(vk_device, &vk_fence_creat_info, nullptr, &s_vk_compute_fence) != VkResult::VK_SUCCESS)
                KB_CORE_ASSERT(false, "[vk::compute_pipeline]: failed to create vkFence!");
        }

        vkWaitForFences(vk_device, 1, &s_vk_compute_fence, VK_TRUE, UINT64_MAX);
        vkResetFences(vk_device, 1, &s_vk_compute_fence);

        VkSubmitInfo vk_compute_submit_queue{};
        vk_compute_submit_queue.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        vk_compute_submit_queue.commandBufferCount = 1;
        vk_compute_submit_queue.pCommandBuffers = &m_vk_active_command_buffer;
        if(vkQueueSubmit(vk_compute_queue, 1, &vk_compute_submit_queue, s_vk_compute_fence) != VkResult::VK_SUCCESS)
            KB_CORE_ASSERT(false, "[vk::compute_pipeline]: failed to submit compute queue!");

        // wait for execution of the shader to complete
        // #TODO this is here for "safety", but blocks our thread...
        vkWaitForFences(vk_device, 1, &s_vk_compute_fence, VK_TRUE, UINT64_MAX);
    }

    m_vk_active_command_buffer = nullptr;
}

void compute_pipeline::dispatch(const glm::uvec3& p_work_group_count) const
{
    KB_CORE_ASSERT(m_vk_active_command_buffer, "[vk::compute_pipeline]: trying to dispatch a pipeline but the active command buffer is null?");

    // call vulkan api to dispatch compute command buffer
    vkCmdDispatch(m_vk_active_command_buffer, p_work_group_count.x, p_work_group_count.y, p_work_group_count.z);
}

void compute_pipeline::set_push_constants(Kablunk::Buffer p_constants)
{
    // call vulkan api to add push constants
    vkCmdPushConstants(
        m_vk_active_command_buffer, 
        m_vk_compute_pipeline_layout, 
        VK_SHADER_STAGE_COMPUTE_BIT, 
        0, 
        p_constants.size(), 
        p_constants.get()
    );
}

void compute_pipeline::create_pipeline()
{
    Kablunk::render::submit(
        [instance = Kablunk::ref{ this }]() mutable
        {
            instance->rt_create_pipeline();
        }
    );
}

void compute_pipeline::rt_create_pipeline()
{
    VkDevice vk_device = Kablunk::VulkanContext::Get()->GetDevice()->GetVkDevice();

    auto descriptor_set_layouts = m_shader->GetAllDescriptorSetLayouts();

    VkPipelineLayoutCreateInfo vk_pipeline_layout_create_info{};
    vk_pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vk_pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
    vk_pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts.data();

    const auto& push_constant_ranges = m_shader->GetPushConstantRanges();
    std::vector<VkPushConstantRange> vulkan_push_constant_ranges{ push_constant_ranges.size() };

    if (!push_constant_ranges.empty())
    {
        // create `VkPushConstantRange` objects
        for (uint32_t i = 0; i < push_constant_ranges.size(); ++i)
        {
            const auto& push_constant_range = push_constant_ranges[i];
            auto& vk_push_constant_range = vulkan_push_constant_ranges[i];

            vk_push_constant_range.stageFlags = push_constant_range.shader_stage;
            vk_push_constant_range.offset = push_constant_range.offset;
            vk_push_constant_range.size = push_constant_range.size;
        }
        
        vk_pipeline_layout_create_info.pushConstantRangeCount = static_cast<uint32_t>(vulkan_push_constant_ranges.size());
        vk_pipeline_layout_create_info.pPushConstantRanges = vulkan_push_constant_ranges.data();
    }

    // call vulkan api to create pipeline layout
    if (vkCreatePipelineLayout(
        vk_device, 
        &vk_pipeline_layout_create_info, 
        nullptr, 
        &m_vk_compute_pipeline_layout) != VkResult::VK_SUCCESS
    )
    {
        KB_CORE_ASSERT(false, "[vk::compute_shader]: failed to create pipeline layout!");
    }

    VkPipelineCacheCreateInfo vk_pipeline_cache_create_info{};
    vk_pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    // call vulkan api to create pipeline cache
    if (vkCreatePipelineCache(
        vk_device,
        &vk_pipeline_cache_create_info,
        nullptr,
        &m_vk_compute_pipeline_cache) != VkResult::VK_SUCCESS)
    {
        KB_CORE_ASSERT(false, "[vk::compute_shader]: failed to create pipeline cache!");
    }

    VkComputePipelineCreateInfo vk_compute_pipeline_create_info{};
    vk_compute_pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    vk_compute_pipeline_create_info.layout = m_vk_compute_pipeline_layout;
    vk_compute_pipeline_create_info.flags = 0;
    const auto& shader_stages = m_shader->GetPipelineShaderStageCreateInfos();
    vk_compute_pipeline_create_info.stage = shader_stages[0];

    // call vulkan api to create pipeline
    if (vkCreateComputePipelines(
        vk_device,
        m_vk_compute_pipeline_cache,
        1,
        &vk_compute_pipeline_create_info,
        nullptr,
        &m_vk_compute_pipeline) != VkResult::VK_SUCCESS)
    {
        KB_CORE_ASSERT(false, "[vk::compute_shader]: failed to create pipeline cache!");
    }
}

}
