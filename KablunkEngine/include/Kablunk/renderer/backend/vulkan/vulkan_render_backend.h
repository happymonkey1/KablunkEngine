#pragma once

#include "Kablunk/renderer/backend/render_backend.h"
#include "Kablunk/renderer/backend/graphics_context.h"
#include "Kablunk/renderer/backend/vulkan/vulkan_index_buffer.h"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

// Forward declaration
class vulkan_context;

// #TODO need to evaluate whether the refs in each command can be passed by const&

class vulkan_render_backend final : public render_backend
{
public:
    vulkan_render_backend() = default;
    vulkan_render_backend(weak_ptr<vulkan_context> p_graphics_context) noexcept;
    ~vulkan_render_backend() noexcept override;

    vulkan_render_backend(const vulkan_render_backend&) noexcept = delete;
    auto operator=(const vulkan_render_backend&) noexcept -> vulkan_render_backend & = delete;
    vulkan_render_backend(vulkan_render_backend&&) noexcept = delete;
    auto operator=(vulkan_render_backend&&) noexcept -> vulkan_render_backend & = delete;

    void init() noexcept override;
    void shutdown() noexcept override;
    void begin_frame() noexcept override;
    void end_frame() noexcept override;
    void begin_render_pass(
        const arc<render_command_buffer>& p_render_command_buffer,
        const arc<render_pass>& p_render_pass,
        bool p_explicit_clear
    ) noexcept override;

    void end_render_pass(const arc<render_command_buffer>& p_render_command_buffer) noexcept override;

    void set_line_width(
        const arc<render_command_buffer>& p_render_command_buffer,
        f32 line_width
    ) noexcept override;

    void submit_fullscreen_quad(
        const arc<render_command_buffer>& p_render_command_buffer,
        const arc<pipeline>& p_pipeline,
        const arc<material>& p_material
    ) noexcept override;

    // geometry rendering

    void render_geometry(
        const arc<render_command_buffer>& p_render_command_buffer,
        const arc<pipeline>& p_pipeline,
        const arc<material>& p_material,
        const arc<vertex_buffer>& p_vertex_buffer,
        const arc<index_buffer>& p_index_buffer,
        const glm::mat4& p_transform,
        uint32_t p_index_count = 0
    ) noexcept override;

    void render_static_mesh(
        const arc<render_command_buffer>& p_render_command_buffer,
        const arc<pipeline>& p_pipeline,
        const arc<Mesh>& p_mesh,
        const arc<MeshData>& p_mesh_data,
        u32 p_sub_mesh_index,
        const arc<material_table>& p_material_table,
        const arc<vertex_buffer>& p_transform_buffer,
        u32 p_transform_offset,
        u32 p_instance_count
    ) noexcept override;

    void render_instanced_sub_mesh(
        arc<render_command_buffer> p_render_command_buffer,
        arc<pipeline> p_pipeline,
        arc<Mesh> p_mesh,
        u32 p_index,
        arc<material_table> p_material_table,
        arc<vertex_buffer> p_transform_buffer,
        u32 p_transform_offset,
        u32 p_bone_transforms_offset,
        u32 p_instance_count
    ) noexcept override;

    void copy_image(
        arc<render_command_buffer> p_render_command_buffer,
        arc<image_2d> p_source_image,
        arc<image_2d> p_destination_image
    ) noexcept override;

    static auto rt_allocate_descriptor_set(
        VkDescriptorSetAllocateInfo& p_alloc_info
    ) noexcept -> VkDescriptorSet;
    static auto rt_allocate_material_descriptor_set(
        weak_ptr<graphics_context> p_context,
        VkDescriptorSetAllocateInfo& p_alloc_info
    ) noexcept -> VkDescriptorSet;

private:
    weak_ptr<vulkan_context> m_vulkan_context = nullptr;
};

} // end namespace kb::render::backend::vk
