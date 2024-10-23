#pragma once

#include "Kablunk/Core/Core.h"

#include "Kablunk/Renderer/render_backend.h"

#include <glm/glm.hpp>

// forward declarations
struct VkDescriptorSetAllocateInfo;
struct VkDescriptorSet_T;
typedef VkDescriptorSet_T* VkDescriptorSet;

namespace kb::render
{ // start namespace kb::render

// #TODO need to evaluate whether the refs in each command can be passed by const&

class vulkan_render_backend : public render_backend<vulkan_render_backend>
{
public:
    vulkan_render_backend() noexcept = default;
    ~vulkan_render_backend() noexcept = default;

    vulkan_render_backend(const vulkan_render_backend&) noexcept = delete;
    vulkan_render_backend(vulkan_render_backend&&) noexcept = delete;

    static auto init() noexcept -> void;
    static auto shutdown() noexcept -> void;
    static auto begin_frame() noexcept -> void;
    static auto end_frame() noexcept -> void;
    static auto begin_render_pass(
        arc<RenderCommandBuffer> p_render_command_buffer,
        arc<render_pass> p_render_pass,
        bool p_explicit_clear
    ) noexcept -> void;

    static auto end_render_pass(arc<RenderCommandBuffer> p_render_command_buffer) noexcept -> void;

    static auto set_line_width(
        arc<RenderCommandBuffer> render_command_buffer,
        f32 line_width
    ) noexcept -> void;

    static auto submit_fullscreen_quad(
        arc<RenderCommandBuffer> p_render_command_buffer,
        arc<Pipeline> p_pipeline,
        arc<Material> p_material
    ) noexcept -> void;

    // geometry rendering

    static auto render_geometry(
        arc<RenderCommandBuffer> p_render_command_buffer,
        arc<Pipeline> p_pipeline,
        arc<Material> p_material,
        arc<VertexBuffer> p_vertex_buffer,
        arc<IndexBuffer> p_index_buffer,
        const glm::mat4& p_transform,
        uint32_t p_index_count = 0
    ) noexcept -> void;

    static auto render_instanced_submesh(
        arc<RenderCommandBuffer> p_render_command_buffer,
        arc<Pipeline> p_pipeline,
        arc<Mesh> p_mesh,
        u32 p_index,
        arc<MaterialTable> p_material_table,
        arc<VertexBuffer> p_transform_buffer,
        u32 p_transform_offset,
        u32 p_bone_transforms_offset,
        u32 p_instance_count
    ) noexcept -> void;

    static auto copy_image(
        arc<RenderCommandBuffer> p_render_command_buffer,
        arc<Image2D> p_source_image,
        arc<Image2D> p_destination_image
    ) noexcept -> void;

    static auto rt_allocate_descriptor_set(VkDescriptorSetAllocateInfo& p_alloc_info) noexcept -> VkDescriptorSet;
    static auto rt_allocate_material_descriptor_set(
        VkDescriptorSetAllocateInfo& p_alloc_info
    ) noexcept -> VkDescriptorSet;

    auto operator=(const vulkan_render_backend&) noexcept -> vulkan_render_backend& = delete;
    auto operator=(vulkan_render_backend&&) noexcept -> vulkan_render_backend& = delete;
};

} // end namespace kb::render
