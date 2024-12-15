#pragma once

#include "Kablunk/Renderer/backend/shader.h"
#include "Kablunk/Renderer/backend/texture.h"
#include "Kablunk/Renderer/backend/pipeline.h"
#include "Kablunk/Renderer/backend/material.h"
#include "Kablunk/Renderer/backend/render_pass.h"
#include "Kablunk/renderer/backend/render_command_buffer.h"
#include "Kablunk/renderer/backend/graphics_context.h"
#include "Kablunk/renderer/Mesh.h"
#include "Kablunk/renderer/backend/backend_type.h"


namespace kb::render::backend
{ // start namespace kb::render::backend

class render_backend
{
public:
    virtual ~render_backend() noexcept = default;

    virtual void init() noexcept = 0;
    virtual void shutdown() noexcept = 0;

    virtual void begin_frame() noexcept = 0;
    virtual void end_frame() noexcept = 0;

    virtual void begin_render_pass(
        const arc<render_command_buffer>& p_render_command_buffer,
        const arc<render_pass>& p_render_pass,
        bool p_explicit_clear = false
    ) = 0;

    virtual void end_render_pass(const arc<render_command_buffer>& p_render_command_buffer) noexcept = 0;

    virtual void set_line_width(const arc<render_command_buffer>& p_render_command_buffer, f32 p_line_width) noexcept = 0;

    virtual void submit_fullscreen_quad(
        const arc<render_command_buffer>& p_render_command_buffer,
        const arc<pipeline>& p_pipeline,
        const arc<material>& p_material
    ) noexcept = 0;

    virtual void render_geometry(
        const arc<render_command_buffer>& p_render_command_buffer,
        const arc<pipeline>& p_pipeline,
        const arc<material>& p_material,
        const arc<vertex_buffer>& p_vertex_buffer,
        const arc<index_buffer>& p_index_buffer,
        const glm::mat4& p_transform,
        uint32_t p_index_count = 0
    ) noexcept = 0;

    // Render a static mesh which has no skeletal animations and a flattened hierarchy
    virtual void render_static_mesh(
        const arc<render_command_buffer>& p_render_command_buffer,
        const arc<pipeline>& p_pipeline,
        const arc<Mesh>& p_mesh,
        const arc<MeshData>& p_mesh_data,
        u32 p_sub_mesh_index,
        const arc<material_table>& p_material_table,
        const arc<vertex_buffer>& p_transform_buffer,
        u32 p_transform_offset,
        u32 p_instance_count
    ) noexcept = 0;

    virtual void render_instanced_sub_mesh_with_material(
        const arc<render_command_buffer>& p_render_command_buffer,
        const arc<pipeline>& p_pipeline,
        const arc<Mesh>& p_mesh,
        u32 p_sub_mesh_index,
        const arc<material>& p_material,
        const arc<vertex_buffer>& p_transform_buffer,
        u32 p_transform_offset,
        u32 p_bone_transforms_offset,
        u32 p_instance_count
    ) noexcept = 0;

    virtual void render_instanced_sub_mesh(
        arc<render_command_buffer> p_render_command_buffer,
        arc<pipeline> p_pipeline,
        arc<Mesh> p_mesh,
        u32 p_index,
        arc<material_table> p_material_table,
        arc<vertex_buffer> p_transform_buffer,
        u32 p_transform_offset,
        u32 p_bone_transforms_offset,
        u32 p_instance_count
    ) noexcept = 0;

    virtual void copy_image(
        arc<render_command_buffer> p_render_command_buffer,
        arc<image_2d> p_source_image,
        arc<image_2d> p_destination_image
    ) noexcept = 0;

private:
    // Factory function to create a render backend
    static auto create(
        render_backend_type_t p_render_backend_type,
        graphics_context* p_graphics_context
    ) noexcept -> render_backend*;

private:
    friend class ::kb::render::Renderer;
};


} // end namespace kb::render
