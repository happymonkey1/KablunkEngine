#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_COMPUTE_SHADER_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_COMPUTE_SHADER_H

#include "Kablunk/renderer/backend/compute_pipeline.h"
#include "kablunk/renderer/backend/vulkan/vulkan_shader.h"
#include "kablunk/renderer/backend/vulkan/VulkanTexture.h"
#include "kablunk/renderer/backend/vulkan/vulkan_render_command_buffer.h"

#include <vulkan/vulkan.h>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

class vulkan_compute_pipeline : public compute_pipeline
{
public:
    // overloaded constructor to construct a compute pipeline from a (compute) shader
    vulkan_compute_pipeline(arc<shader> compute_shader);

    // execute the compute pipeline
    void execute(
        VkDescriptorSet* p_descriptor_sets,
        uint32_t p_descriptor_set_count,
        uint32_t p_group_count_x,
        uint32_t p_group_count_y,
        uint32_t p_group_count_z
    );

    // ==================================
    // render::compute_pipeline interface
    // ==================================

    void begin(arc<render_command_buffer> p_render_command_buffer = {}) override;
    void rt_begin(arc<render_command_buffer> p_render_command_buffer = {}) override;
    void end() override;
    arc<shader> get_shader() const override { return static_cast<arc<shader>>(m_shader); }

    // ==================================

    // dispatch compute shaders to gpu
    void dispatch(const glm::uvec3& p_work_group_count) const;

    // return the active command buffer
    VkCommandBuffer get_active_command_buffer() { return m_vk_active_command_buffer; }
    // return the pipeline layout
    VkPipelineLayout get_layout() const { return m_vk_compute_pipeline_layout; }

    // set push constants
    void set_push_constants(owning_buffer p_constants);
    void create_pipeline();
private:
    void rt_create_pipeline();
private:
    // arc to the underlying shader
    arc<vulkan_shader> m_shader;
    // pipeline layout
    VkPipelineLayout m_vk_compute_pipeline_layout = nullptr;
    // pipeline cache
    VkPipelineCache m_vk_compute_pipeline_cache = nullptr;
    // compute pipeline
    VkPipeline m_vk_compute_pipeline = nullptr;
    // active command buffer
    VkCommandBuffer m_vk_active_command_buffer = nullptr;
    // flag for whether the pipeline is using the graphics queue
    bool m_using_graphics_queue = false;
};

} // end namespace kb::render::backend::vk

#endif
