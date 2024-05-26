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

// forward declaration
struct vulkan_render_backend_data;

class vulkan_render_backend : public render_backend<vulkan_render_backend>
{
public:
    vulkan_render_backend() noexcept = default;
    ~vulkan_render_backend() noexcept = default;

    auto init() noexcept -> void;
    auto shutdown() noexcept -> void;

    auto begin_frame() noexcept -> void;
    auto end_frame() noexcept -> void;

    auto begin_render_pass(
        ref<RenderCommandBuffer> p_render_command_buffer,
        ref<render_pass> p_render_pass,
        bool p_explicit_clear
    ) noexcept -> void;

    auto end_render_pass(ref<RenderCommandBuffer> p_render_command_buffer) noexcept -> void;

    auto set_line_width(
        ref<RenderCommandBuffer> render_command_buffer,
        f32 line_width
    ) noexcept -> void;

    auto submit_fullscreen_quad(
        ref<RenderCommandBuffer> p_render_command_buffer,
        ref<Pipeline> p_pipeline,
        ref<Material> p_material
    ) noexcept -> void;

    // geometry rendering

    static auto render_geometry(
        ref<RenderCommandBuffer> p_render_command_buffer,
        ref<Pipeline> p_pipeline,
        ref<Material> p_material,
        ref<VertexBuffer> p_vertex_buffer,
        ref<IndexBuffer> p_index_buffer,
        const glm::mat4& p_transform,
        uint32_t p_index_count = 0
    ) noexcept -> void;

    auto render_instanced_submesh(
        ref<RenderCommandBuffer> p_render_command_buffer,
        ref<Pipeline> p_pipeline,
        ref<Mesh> p_mesh,
        u32 p_index,
        ref<MaterialTable> p_material_table,
        ref<VertexBuffer> p_transform_buffer,
        u32 p_transform_offset,
        u32 p_bone_transforms_offset,
        u32 p_instance_count
    ) noexcept -> void;

    auto copy_image(
        ref<RenderCommandBuffer> p_render_command_buffer,
        ref<Image2D> p_source_image,
        ref<Image2D> p_destination_image
    ) noexcept -> void;

    auto rt_allocate_descriptor_set(VkDescriptorSetAllocateInfo& p_alloc_info) const noexcept -> VkDescriptorSet;
    auto rt_allocate_material_descriptor_set(
        VkDescriptorSetAllocateInfo& p_alloc_info
    ) const noexcept -> VkDescriptorSet;
};

} // end namespace kb::render
