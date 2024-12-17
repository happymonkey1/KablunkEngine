#pragma once
#ifndef KABLUNK_RENDERER_SCENE_RENDERER_H
#define KABLUNK_RENDERER_SCENE_RENDERER_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Math/vec.hpp"

#include "Kablunk/Scene/Scene.h"

#include "Kablunk/renderer/backend/image.h"
#include "Kablunk/renderer/backend/render_pass.h"
#include "Kablunk/renderer/backend/pipeline.h"
#include "Kablunk/renderer/backend/render_command_buffer.h"
#include "Kablunk/renderer/backend/material.h"
#include "Kablunk/renderer/backend/uniform_buffer_set.h"
#include "Kablunk/renderer/backend/storage_buffer_set.h"
#include "Kablunk/renderer/material_asset.h"
#include "Kablunk/renderer/Mesh.h"

#include <string_view>

namespace kb
{
class scene_renderer_panel;
}

// forward declaration
namespace kb::ui
{
class IPanel;
}

namespace kb::render
{ // start namespace kb::render

// TODO: should be private
struct mesh_transform_handle
{
    mesh_handle m_mesh_handle;
    // TODO: material?
    u32 m_sub_mesh_index;

    auto operator<=>(const mesh_transform_handle&) const = default;
};

} // end namespace kb::render

namespace std
{ // start namespace std

template <>
struct std::hash<kb::render::mesh_transform_handle>
{
    std::size_t operator()(const kb::render::mesh_transform_handle& p_item_handle) const noexcept
    {
        kb::u64 hash = p_item_handle.m_mesh_handle.as<kb::u32>();
        hash |= static_cast<kb::u64>(p_item_handle.m_sub_mesh_index) << 32;
        return hash;
    }
};

} // end namespace std

namespace kb::render
{ // start namespace kb::render

// forward declaration
class renderer_2d;

constexpr size_t k_max_cascades = 4ull;

struct scene_renderer_specification_t
{
	bool swap_chain_target = false;
};

struct camera_data_ub_t
{
	glm::mat4 view_projection;
	glm::mat4 projection;
	glm::mat4 view;
	vec3_packed position;
};

struct scene_renderer_camera_t
{
	camera camera;
	glm::mat4 view_mat;
    f32 m_near_clip = 0.1f;
    f32 m_far_clip = 1000.f;
};

struct scene_renderer_data_ub_t
{
	scene_renderer_camera_t camera;
	light_environment_data_t light_environment;
};

struct shadow_cascade_data_t
{
    f32 m_split_depth;
    glm::mat4 m_view_projection;
    glm::mat4 m_view;
};

struct shadow_cascade_data_ub_t
{
    glm::mat4 m_view_projection[4];
};

struct point_light_ub_t
{
    static constexpr size_t k_point_light_buffer_size = 128ull;
    uint32_t count{ 0 };
    vec3_packed padding{};
    point_light_t point_lights[k_point_light_buffer_size]{};
};

struct renderer_data_ub_t
{
    glm::vec4 m_cascade_splits{ 0.f };
};

struct scene_renderer_statistics_t
{
    u32 m_draw_call_count = 0;
    u32 m_vertices_count = 0;
    u32 m_indices_count = 0;
    u32 m_triangle_count = 0;
};

class scene_renderer final : public RefCounted
{
public:
    scene_renderer(const arc<Scene>& context, const scene_renderer_specification_t& spec = {});
    ~scene_renderer() override;

    // #TODO this should be private, if construct is only place that calls this
    void init();
    void set_scene(arc<Scene> context);

    void begin_scene(const scene_renderer_camera_t& camera);
    void end_scene();

    void submit_mesh(
        const arc<Mesh>& mesh,
        const arc<material_table>& material_table,
        const glm::mat4& transform = glm::mat4{ 1.0f },
        const arc<backend::material>& override_material = {}
    );

    void set_multi_threaded(bool threaded) { m_use_threads = threaded; }
    bool is_multi_threaded() const { return m_use_threads; }

    void set_viewport_size(uint32_t width, uint32_t height);
    arc<backend::render_pass> get_final_render_pass();
    arc<backend::render_pass> get_composite_render_pass() { return m_composite_pass; }
    arc<backend::frame_buffer> get_external_composite_frame_buffer() { return m_composite_pass->get_target_frame_buffer(); }
    arc<backend::image_2d> get_final_render_pass_image();

    void on_imgui_render(const arc<renderer_2d>& p_renderer_2d);

    static void wait_for_threads();

private:
    auto submit_uniform_buffers() noexcept -> void;
    void flush_draw_list();
    void pre_render();
    void clear_pass();
    auto shadow_pass() noexcept -> void;
    void geometry_pass();
    void composite_pass();

    void clear_pass(arc<backend::render_pass> render_pass, bool explicit_clear = false);

    auto calculate_shadow_map_data(
        shadow_cascade_data_t* p_cascades_data,
        const scene_renderer_camera_t& p_scene_camera,
        const glm::vec3& p_light_direction
    ) noexcept -> void;

private:
    arc<Scene> m_context;
    scene_renderer_specification_t m_specification;

    arc<backend::render_command_buffer> m_command_buffer;

    arc<backend::render_pass> m_directional_shadow_pass[4];
    arc<backend::render_pass> m_geometry_pass;
    arc<backend::render_pass> m_composite_pass;

    arc<backend::material> m_composite_material;
    arc<backend::material> m_dir_shadow_pass_material;

#if 0
    arc<render::render_pass> m_external_composite_render_pass;
    arc<render::frame_buffer> m_external_composite_frame_buffer{};
#endif

    struct gpu_time_query_indices_t
    {
        u32 m_shadow_pass_query;
        uint32_t m_geometry_pass_query;
        uint32_t m_composite_pass_query;
    };

    arc<backend::texture_2d> m_bloom_texture;
    arc<backend::texture_2d> m_bloom_dirt_texture;

    struct transform_vertex_data_t
    {
        vec4_packed MRow[3];
    };

    static_assert(sizeof(transform_vertex_data_t) == 48);

    arc<backend::vertex_buffer> m_transform_buffer;
    transform_vertex_data_t* m_transform_vertex_data = nullptr;

    struct transform_map_data_t
    {
        std::vector<transform_vertex_data_t> m_transforms;
        u32 m_transform_offset = 0;
    };

    unordered_flat_map<mesh_transform_handle, transform_map_data_t> m_transform_map{};

    arc<backend::uniform_buffer_set> m_renderer_data_uniform_buffer_set{};
    arc<backend::uniform_buffer_set> m_camera_uniform_buffer_set{};
    arc<backend::uniform_buffer_set> m_point_lights_uniform_buffer_set{};
    arc<backend::uniform_buffer_set> m_directional_light_set{};
    arc<backend::uniform_buffer_set> m_shadow_data_uniform_buffer_set{};
	arc<backend::storage_buffer_set> m_storage_buffer_set;

    point_light_ub_t* m_point_lights_ub = new point_light_ub_t{};

    // statistics
	gpu_time_query_indices_t m_gpu_time_query_indices;
    scene_renderer_statistics_t m_statistics;

    u32 m_viewport_width = 0, m_viewport_height = 0;
	bool m_active = false;
	bool m_needs_resize = false;
	bool m_resources_created = false;

	// flag for flushing scene data on a separate "job" thread
	bool m_use_threads = false;

    struct
    {
        f32 m_shadow_scale_from_origin = 0.f;
        f32 m_cascade_split_lambda = 0.92f;
        glm::vec4 m_cascade_splits{};
        f32 m_cascade_far_plane_offset = 50.0f;
        f32 m_cascade_near_plane_offset = -50.f;
        f32 m_shadow_cascade_splits[k_max_cascades];
        f32 m_use_manual_cascade_splits = false;
    } m_shadow_cascade_data;

	scene_renderer_data_ub_t m_scene_data;

	struct draw_command_data_t
	{
		arc<Mesh> Mesh{};
		u32 Submesh_index = 0;
		arc<material_table> Material_table{};
		arc<backend::material> Override_material{};

        u32 Instance_count = 0;
        u32 Instance_offset = 0;
	};

    unordered_flat_map<mesh_transform_handle, draw_command_data_t> m_draw_list{};

	// =================

	friend class VulkanRenderer2D;
    friend class ::kb::scene_renderer_panel;
};

} // end namespace kb::render

#endif
