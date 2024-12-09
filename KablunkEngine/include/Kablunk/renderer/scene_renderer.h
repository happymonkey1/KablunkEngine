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

// forward declaration
namespace kb::ui
{
class IPanel;
}

namespace kb::render
{ // start namespace kb::render

// forward declaration
class renderer_2d;

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
};

struct scene_renderer_data_t
{
	scene_renderer_camera_t camera;
	LightEnvironmentData light_environment;
};

struct point_light_ub_t
{
    static constexpr size_t k_point_light_buffer_size = 128ull;
    uint32_t count{ 0 };
    vec3_packed padding{};
    point_light_t point_lights[k_point_light_buffer_size]{};
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

	void submit_mesh(arc<Mesh> mesh, uint32_t submesh_index, arc<material_table> material_table, const glm::mat4& transform = glm::mat4{ 1.0f }, arc<backend::material> override_material = {});

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
	void flush_draw_list();
	void pre_render();
	void clear_pass();
	void geometry_pass();
	void composite_pass();

	void clear_pass(arc<backend::render_pass> render_pass, bool explicit_clear = false);

private:
	arc<Scene> m_context;
	scene_renderer_specification_t m_specification;

	arc<backend::render_command_buffer> m_command_buffer;

	arc<backend::render_pass> m_geometry_pass;
	arc<backend::render_pass> m_composite_pass;

	arc<backend::material> m_composite_material;

#if 0
	arc<render::render_pass> m_external_composite_render_pass;
    arc<render::frame_buffer> m_external_composite_frame_buffer{};
#endif

	struct GPUTimeQueryIndices
	{
		uint32_t geometry_pass_query;
		uint32_t composite_pass_query;
	};

	arc<backend::texture_2d> m_bloom_texture;
	arc<backend::texture_2d> m_bloom_dirt_texture;

	struct transform_vertex_data_t
	{
		vec4_packed MRow[3];
	};

	arc<backend::vertex_buffer> m_transform_buffer;
	transform_vertex_data_t* m_transform_vertex_data = nullptr;

    arc<backend::uniform_buffer_set> m_camera_uniform_buffer_set{};
    arc<backend::uniform_buffer_set> m_point_lights_uniform_buffer_set{};
	arc<backend::storage_buffer_set> m_storage_buffer_set;

    point_light_ub_t* m_point_lights_ub = new point_light_ub_t{};

	GPUTimeQueryIndices m_gpu_time_query_indices;

	uint32_t m_viewport_width = 0, m_viewport_height = 0;
	bool m_active = false;
	bool m_needs_resize = false;
	bool m_resources_created = false;

	// flag for flushing scene data on a separate "job" thread
	bool m_use_threads = false;

	scene_renderer_data_t m_scene_data;

	struct draw_command_data_t
	{
		arc<Mesh> Mesh;
		uint32_t Submesh_index;
		arc<material_table> Material_table;
		arc<backend::material> Override_material;

		uint32_t Instance_count = 0;
		uint32_t Instance_offset = 0;
		glm::mat4 Transform; // #TODO store separately in a map that maps MeshKey to transforms
	};

	// #TODO MeshKeys
	// Mesh Keys store AssetHandles to the mesh data and material handle, as well as the submesh index of the mesh
	// implement a operator< so they can be sorted into a map

	// #TODO replace with a map that maps MeshKeys to DrawCommandData
	std::vector<draw_command_data_t> m_draw_list;

	// =================

	friend class VulkanRenderer2D;
};

} // end namespace kb::render

#endif
