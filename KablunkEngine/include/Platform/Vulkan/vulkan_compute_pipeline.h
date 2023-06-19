#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_COMPUTE_SHADER_H
#define KABLUNK_PLATFORM_VULKAN_COMPUTE_SHADER_H

#include "Kablunk/Renderer/compute_pipeline.h"

#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/VulkanRenderCommandBuffer.h"

#include <vulkan/vulkan.h>

namespace kb::vk
{ // start namespace kb::vk
    
class compute_pipeline : public render::compute_pipeline
{
public:
    // overloaded constructor to construct a compute pipeline from a (compute) shader
    compute_pipeline(Kablunk::ref<Kablunk::Shader> compute_shader);

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

    virtual void begin(Kablunk::ref<Kablunk::RenderCommandBuffer> p_render_command_buffer = nullptr) override;
    virtual void rt_begin(Kablunk::ref<Kablunk::RenderCommandBuffer> p_render_command_buffer = nullptr) override;
    virtual void end() override;
    virtual Kablunk::ref<Kablunk::Shader> get_shader() const override { return m_shader; }

    // ==================================

    // dispatch compute shaders to gpu
    void dispatch(const glm::uvec3& p_work_group_count) const;

    // return the active command buffer
    VkCommandBuffer get_active_command_buffer() { return m_vk_active_command_buffer; }
    // return the pipeline layout
    VkPipelineLayout get_layout() const { return m_vk_compute_pipeline_layout; }
    
    // set push constants
    void set_push_constants(Kablunk::Buffer p_constants);
    void create_pipeline();
private:
    void rt_create_pipeline();
private:
    // ref to the underlying shader
    Kablunk::ref<Kablunk::VulkanShader> m_shader;
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

} // end namespace kb::vk

#endif
