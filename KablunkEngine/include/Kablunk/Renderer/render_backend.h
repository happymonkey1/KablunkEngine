#pragma once

#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/Pipeline.h"
#include "Kablunk/Renderer/Material.h"
#include "Kablunk/Renderer/RendererAPI.h"

namespace kb::render
{ // start namespace kb::render

enum class render_backend_type_t : u8
{
    vulkan = 0,
};

template <typename RenderBackend>
struct render_backend
{
    auto init() noexcept -> void { backend()->init(); }
    auto shutdown() noexcept -> void { backend()->shutdown(); }

    auto begin_frame() noexcept -> void { backend()->begin_frame(); }
    auto end_frame() noexcept -> void { backend()->end_frame(); }

    auto begin_render_pass(
        arc<RenderCommandBuffer> p_render_command_buffer,
        arc<render_pass> p_render_pass,
        bool p_explicit_clear = false
    ) noexcept -> void
    {
        backend()->begin_render_pass(p_render_command_buffer, p_render_pass, p_explicit_clear);
    }

    auto end_render_pass(arc<RenderCommandBuffer> p_render_command_buffer) noexcept -> void
    {
        backend()->end_render_pass(p_render_command_buffer);
    }

    auto set_line_width(arc<RenderCommandBuffer> p_render_command_buffer, f32 p_line_width) noexcept -> void
    {
        backend()->set_line_width(p_render_command_buffer, p_line_width);
    }

    auto submit_fullscreen_quad(
        arc<RenderCommandBuffer> p_render_command_buffer,
        arc<Pipeline> p_pipeline,
        arc<Material> p_material
    ) noexcept -> void
    {
        backend()->submit_fullscreen_quad(p_render_command_buffer, p_pipeline, p_material);
    }

    auto render_geometry(
        arc<RenderCommandBuffer> p_render_command_buffer,
        arc<Pipeline> p_pipeline,
        arc<Material> p_material,
        arc<VertexBuffer> p_vertex_buffer,
        arc<IndexBuffer> p_index_buffer,
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
        arc<RenderCommandBuffer> p_render_command_buffer,
        arc<Pipeline> p_pipeline,
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
        arc<RenderCommandBuffer> p_render_command_buffer,
        arc<Image2D> p_source_image,
        arc<Image2D> p_destination_image
    ) noexcept -> void
    {
        backend()->copy_image(p_render_command_buffer, p_source_image, p_destination_image);
    }

    auto backend() noexcept -> RenderBackend* { return static_cast<RenderBackend*>(this); }
    auto backend() const noexcept -> RenderBackend* { return static_cast<RenderBackend*>(this); }
};

} // end namespace kb::render
