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
        const ref<RenderCommandBuffer>& p_render_command_buffer,
        const ref<render_pass>& p_render_pass,
        bool p_explicit_clear
    ) noexcept -> void;

    auto end_render_pass(const ref<RenderCommandBuffer>& p_render_command_buffer) noexcept -> void;

    auto set_line_width(
        const ref<RenderCommandBuffer>& render_command_buffer,
        f32 line_width
    ) noexcept -> void;

    auto submit_fullscreen_quad(
        const ref<RenderCommandBuffer>& p_render_command_buffer,
        const ref<Pipeline>& p_pipeline,
        const ref<Material>& p_material
    ) noexcept -> void;

    // geometry rendering

    static auto render_geometry(
        const ref<RenderCommandBuffer>& p_render_command_buffer,
        const ref<Pipeline>& p_pipeline,
        const ref<Material>& p_material,
        const ref<VertexBuffer>& p_vertex_buffer,
        const ref<IndexBuffer>& p_index_buffer,
        const glm::mat4& p_transform,
        uint32_t p_index_count = 0
    ) noexcept -> void;

    auto render_instanced_submesh(
        const ref<RenderCommandBuffer>& p_render_command_buffer,
        const ref<Pipeline>& p_pipeline,
        const ref<Mesh>& p_mesh,
        u32 p_index,
        const ref<MaterialTable>& p_material_table,
        const ref<VertexBuffer>& p_transform_buffer,
        u32 p_transform_offset,
        u32 p_bone_transforms_offset,
        u32 p_instance_count
    ) noexcept -> void;

    auto rt_allocate_descriptor_set(VkDescriptorSetAllocateInfo& p_alloc_info) const noexcept -> VkDescriptorSet;

private:
    std::unique_ptr<vulkan_render_backend_data> m_renderer_data{};
};

} // end namespace kb::render
