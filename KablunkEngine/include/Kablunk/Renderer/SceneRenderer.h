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
#include "Kablunk/renderer/MaterialAsset.h"
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

struct SceneRendererSpecification
{
	bool swap_chain_target = false;
};

struct CameraDataUB
{
	glm::mat4 view_projection;
	glm::mat4 projection;
	glm::mat4 view;
	vec3_packed position;
};

struct SceneRendererCamera
{
	Camera camera;
	glm::mat4 view_mat;
};

struct SceneRendererData
{
	SceneRendererCamera camera;
	LightEnvironmentData light_environment;
};

struct PointLightUB
{
    static constexpr size_t k_point_light_buffer_size = 128ull;
    uint32_t count{ 0 };
    vec3_packed padding{};
    PointLight point_lights[k_point_light_buffer_size]{};
};

class SceneRenderer final : public RefCounted
{
public:
	SceneRenderer(const arc<Scene>& context, const SceneRendererSpecification& spec = {});
	~SceneRenderer() override;

    // #TODO this should be private, if construct is only place that calls this
	void init();
	void set_scene(arc<Scene> context);

	void begin_scene(const SceneRendererCamera& camera);
	void end_scene();

	void submit_mesh(arc<Mesh> mesh, uint32_t submesh_index, arc<MaterialTable> material_table, const glm::mat4& transform = glm::mat4{ 1.0f }, arc<backend::material> override_material = {});

	void set_multi_threaded(bool threaded) { m_use_threads = threaded; }
	bool is_multi_threaded() const { return m_use_threads; }

	void set_viewport_size(uint32_t width, uint32_t height);
	arc<backend::render_pass> get_final_render_pass();
	arc<backend::render_pass> get_composite_render_pass() { return m_composite_pass; }
    arc<backend::frame_buffer> get_external_composite_frame_buffer() { return m_composite_pass->get_target_frame_buffer(); }
	arc<backend::image_2d> get_final_render_pass_image();

	void on_imgui_render(const arc<renderer_2d>& p_renderer_2d);

	static void wait_for_threads();

	void submit_ui_panel(ui::IPanel* panel);

private:
	void flush_draw_list();
	void flush_2d_draw_list();
	void pre_render();
	void clear_pass();
	void geometry_pass();
	void composite_pass();

	void clear_pass(arc<backend::render_pass> render_pass, bool explicit_clear = false);

	// draw all ui elements presented to the scene renderer
	void ui_pass();

	// draw all 2d elements presented to the scene renderer
	void two_dimensional_pass();

private:
	arc<Scene> m_context;
	SceneRendererSpecification m_specification;

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

	struct TransformVertexData
	{
		glm::vec4 MRow[3];
	};

	arc<backend::vertex_buffer> m_transform_buffer;
	TransformVertexData* m_transform_vertex_data = nullptr;

    arc<backend::uniform_buffer_set> m_camera_uniform_buffer_set{};
    arc<backend::uniform_buffer_set> m_point_lights_uniform_buffer_set{};
	arc<backend::storage_buffer_set> m_storage_buffer_set;

    PointLightUB* m_point_lights_ub = new PointLightUB{};

	GPUTimeQueryIndices m_gpu_time_query_indices;

	uint32_t m_viewport_width = 0, m_viewport_height = 0;
	bool m_active = false;
	bool m_needs_resize = false;
	bool m_resources_created = false;

	// flag for flushing scene data on a separate "job" thread
	bool m_use_threads = false;

	SceneRendererData m_scene_data;

	struct DrawCommandData
	{
		arc<Mesh> Mesh;
		uint32_t Submesh_index;
		arc<MaterialTable> Material_table;
		arc<backend::material> Override_material;

		uint32_t Instance_count = 0;
		uint32_t Instance_offset = 0;
		glm::mat4 Transform; // #TODO store separately in a map that maps MeshKey to transforms
	};

	// #TODO MeshKeys
	// Mesh Keys store AssetHandles to the mesh data and material handle, as well as the submesh index of the mesh
	// implement a operator< so they can be sorted into a map

	// #TODO replace with a map that maps MeshKeys to DrawCommandData
	std::vector<DrawCommandData> m_draw_list;

	// =========
	// ui panels
	// =========

	std::vector<ui::IPanel*> m_ui_panels_list;

	// =========

	// =================
	// 2d composite data
	// =================

	// list of sprite entities to be drawn in the 2d composite pass
	// #TODO linear allocator 
	std::vector<Entity> m_entity_list;

	// =================

	friend class VulkanRenderer2D;
};

} // end namespace kb::render

#endif
