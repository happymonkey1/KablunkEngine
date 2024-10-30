#pragma once

#include "Kablunk/Renderer/backend/shader.h"
#include "Kablunk/Renderer/backend/texture.h"
#include "Kablunk/Renderer/backend/pipeline.h"
#include "Kablunk/Renderer/backend/material.h"
#include "Kablunk/Renderer/backend/render_pass.h"
#include "Kablunk/renderer/backend/render_command_buffer.h"
#include "Kablunk/renderer/backend/graphics_context.h"
#include "Kablunk/renderer/Mesh.h"


namespace kb::render::backend
{ // start namespace kb::render::backend

template <typename RenderBackend>
struct render_backend
{
    auto init() noexcept -> void { backend()->init(); }
    auto shutdown() noexcept -> void { backend()->shutdown(); }

    auto begin_frame(weak_arc<graphics_context> p_context) noexcept -> void { backend()->begin_frame(p_context); }
    auto end_frame() noexcept -> void { backend()->end_frame(); }

    auto begin_render_pass(
        weak_arc<graphics_context> p_context,
        const arc<render_command_buffer>& p_render_command_buffer,
        const arc<render_pass>& p_render_pass,
        bool p_explicit_clear = false
    ) noexcept -> void
    {
        backend()->begin_render_pass(p_context, p_render_command_buffer, p_render_pass, p_explicit_clear);
    }

    auto end_render_pass(const arc<render_command_buffer>& p_render_command_buffer) noexcept -> void
    {
        backend()->end_render_pass(p_render_command_buffer);
    }

    auto set_line_width(const arc<render_command_buffer>& p_render_command_buffer, f32 p_line_width) noexcept -> void
    {
        backend()->set_line_width(p_render_command_buffer, p_line_width);
    }

    auto submit_fullscreen_quad(
        const arc<render_command_buffer>& p_render_command_buffer,
        const arc<pipeline>& p_pipeline,
        const arc<material>& p_material
    ) noexcept -> void
    {
        backend()->submit_fullscreen_quad(p_render_command_buffer, p_pipeline, p_material);
    }

    auto render_geometry(
        const arc<render_command_buffer>& p_render_command_buffer,
        const arc<pipeline>& p_pipeline,
        const arc<material>& p_material,
        const arc<VertexBuffer>& p_vertex_buffer,
        const arc<IndexBuffer>& p_index_buffer,
        const glm::mat4& p_transform,
        uint32_t p_index_count = 0
    ) noexcept -> void
    {
        backend()->render_geometry(
            p_render_command_buffer,
            p_pipeline,
            p_material,
            p_vertex_buffer,
            p_index_buffer,
            p_transform,
            p_index_count
        );
    }

    auto render_instanced_submesh(
        arc<render_command_buffer> p_render_command_buffer,
        arc<pipeline> p_pipeline,
        arc<Mesh> p_mesh,
        u32 p_index,
        arc<MaterialTable> p_material_table,
        arc<VertexBuffer> p_transform_buffer,
        u32 p_transform_offset,
        u32 p_bone_transforms_offset,
        u32 p_instance_count
    ) noexcept -> void
    {
        backend()->render_instanced_submesh(
            p_render_command_buffer,
            p_pipeline,
            p_mesh,
            p_index,
            p_material_table,
            p_transform_buffer,
            p_transform_offset,
            p_bone_transforms_offset,
            p_instance_count
        );
    }

    auto copy_image(
        arc<render_command_buffer> p_render_command_buffer,
        arc<image_2d> p_source_image,
        arc<image_2d> p_destination_image
    ) noexcept -> void
    {
        backend()->copy_image(p_render_command_buffer, p_source_image, p_destination_image);
    }

    auto backend() noexcept -> RenderBackend* { return static_cast<RenderBackend*>(this); }
    auto backend() const noexcept -> RenderBackend* { return static_cast<RenderBackend*>(this); }

protected:
    // Non-owning (owned by renderer) pointer to graphics context
    graphics_context* m_context = nullptr;

private:
    friend class ::kb::render::Renderer;
};

} // end namespace kb::render
