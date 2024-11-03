#pragma once

#include "Kablunk/renderer/backend/render_backend.h"
#include "Kablunk/renderer/backend/graphics_context.h"

#include <glm/glm.hpp>


// forward declarations
struct VkDescriptorSetAllocateInfo;
struct VkDescriptorSet_T;
typedef VkDescriptorSet_T* VkDescriptorSet;

namespace kb::render::backend::vk
{ // start namespace kb::render::vk

// #TODO need to evaluate whether the refs in each command can be passed by const&

class vulkan_render_backend : public render_backend<vulkan_render_backend>
{
public:
    vulkan_render_backend() noexcept = default;
    ~vulkan_render_backend() noexcept = default;

    vulkan_render_backend(const vulkan_render_backend&) noexcept = delete;
    auto operator=(const vulkan_render_backend&) noexcept -> vulkan_render_backend & = delete;
    vulkan_render_backend(vulkan_render_backend&&) noexcept = delete;
    auto operator=(vulkan_render_backend&&) noexcept -> vulkan_render_backend & = delete;

    static auto init() noexcept -> void;
    static auto shutdown() noexcept -> void;
    static auto begin_frame(
        weak_ptr<graphics_context> p_context
    ) noexcept -> void;
    static auto end_frame() noexcept -> void;
    static auto begin_render_pass(
        weak_ptr<graphics_context> p_graphics_context,
        const arc<render_command_buffer>& p_render_command_buffer,
        const arc<render_pass>& p_render_pass,
        bool p_explicit_clear
    ) noexcept -> void;

    static auto end_render_pass(const arc<render_command_buffer>& p_render_command_buffer) noexcept -> void;

    static auto set_line_width(
        const arc<render_command_buffer>& p_render_command_buffer,
        f32 line_width
    ) noexcept -> void;

    static auto submit_fullscreen_quad(
        const arc<render_command_buffer>& p_render_command_buffer,
        const arc<pipeline>& p_pipeline,
        const arc<material>& p_material
    ) noexcept -> void;

    // geometry rendering

    static auto render_geometry(
        const arc<render_command_buffer>& p_render_command_buffer,
        const arc<pipeline>& p_pipeline,
        const arc<material>& p_material,
        const arc<vertex_buffer>& p_vertex_buffer,
        const arc<index_buffer>& p_index_buffer,
        const glm::mat4& p_transform,
        uint32_t p_index_count = 0
    ) noexcept -> void;

    static auto render_instanced_submesh(
        arc<render_command_buffer> p_render_command_buffer,
        arc<pipeline> p_pipeline,
        arc<Mesh> p_mesh,
        u32 p_index,
        arc<MaterialTable> p_material_table,
        arc<vertex_buffer> p_transform_buffer,
        u32 p_transform_offset,
        u32 p_bone_transforms_offset,
        u32 p_instance_count
    ) noexcept -> void;

    static auto copy_image(
        arc<render_command_buffer> p_render_command_buffer,
        arc<image_2d> p_source_image,
        arc<image_2d> p_destination_image
    ) noexcept -> void;

    static auto rt_allocate_descriptor_set(
        VkDescriptorSetAllocateInfo& p_alloc_info
    ) noexcept -> VkDescriptorSet;
    static auto rt_allocate_material_descriptor_set(
        weak_ptr<graphics_context> p_context,
        VkDescriptorSetAllocateInfo& p_alloc_info
    ) noexcept -> VkDescriptorSet;
};

} // end namespace kb::render::backend::vk
