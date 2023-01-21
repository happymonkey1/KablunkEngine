#pragma once
#ifndef KABLUNK_RENDERER_SCENE_RENDERER_H
#define KABLUNK_RENDERER_SCENE_RENDERER_H

#include "Kablunk/Core/Core.h"

#include "Kablunk/Scene/Scene.h"

#include "Kablunk/Renderer/Image.h"
#include "Kablunk/Renderer/RenderPass.h"
#include "Kablunk/Renderer/Pipeline.h"
#include "Kablunk/Renderer/RenderCommandBuffer.h"
#include "Kablunk/Renderer/Material.h"
#include "Kablunk/Renderer/UniformBufferSet.h"
#include "Kablunk/Renderer/StorageBufferSet.h"
#include "Kablunk/Renderer/MaterialAsset.h"
#include "Kablunk/Renderer/Mesh.h"

namespace Kablunk
{

	// forward declaration
	namespace ui
	{
		class IPanel;
	}

	struct SceneRendererSpecification
	{
		bool swap_chain_target = false;
	};

	struct CameraDataUB
	{
		glm::mat4 view_projection;
		glm::mat4 projection;
		glm::mat4 view;
		glm::vec3 position;
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


	class SceneRenderer : public RefCounted
	{
	public:
		SceneRenderer(const IntrusiveRef<Scene>& context, const SceneRendererSpecification& spec = {});
		~SceneRenderer();

		void init();
		void set_scene(IntrusiveRef<Scene> context);

		void begin_scene(const SceneRendererCamera& camera);
		void end_scene();

		void submit_mesh(IntrusiveRef<Mesh> mesh, uint32_t submesh_index, IntrusiveRef<MaterialTable> material_table, const glm::mat4& transform = glm::mat4{ 1.0f }, IntrusiveRef<Material> override_material = nullptr);

		void set_multi_threaded(bool threaded) { m_use_threads = threaded; }
		bool is_multi_threaded() const { return m_use_threads; }

		void set_viewport_size(uint32_t width, uint32_t height);
		IntrusiveRef<RenderPass> get_final_render_pass();
		IntrusiveRef<RenderPass> get_composite_render_pass() { return m_composite_pipeline->GetSpecification().render_pass; }
		IntrusiveRef<RenderPass> get_external_composite_render_pass() { return m_external_composite_render_pass; }
		IntrusiveRef<Image2D> get_final_render_pass_image();

		void OnImGuiRender();

		static void wait_for_threads();

		void submit_ui_panel(ui::IPanel* panel);

	private:
		void flush_draw_list();
		void flush_2d_draw_list();
		void pre_render();
		void clear_pass();
		void geometry_pass();
		void composite_pass();

		void clear_pass(IntrusiveRef<RenderPass> render_pass, bool explicit_clear = false);
		
		// draw all ui elements presented to the scene renderer
		void ui_pass();

		// draw all 2d elements presented to the scene renderer
		void two_dimensional_pass();
		
	private:
		IntrusiveRef<Scene> m_context;
		SceneRendererSpecification m_specification;

		IntrusiveRef<RenderCommandBuffer> m_command_buffer;

		IntrusiveRef<Pipeline> m_geometry_pipeline;
		IntrusiveRef<Pipeline> m_composite_pipeline;

		IntrusiveRef<Material> m_composite_material;

		IntrusiveRef<RenderPass> m_external_composite_render_pass;

		struct GPUTimeQueryIndices
		{
			uint32_t geometry_pass_query;
			uint32_t composite_pass_query;
		};

		IntrusiveRef<Texture2D> m_bloom_texture;
		IntrusiveRef<Texture2D> m_bloom_dirt_texture;

		struct TransformVertexData
		{
			glm::vec4 MRow[3];
		};
		IntrusiveRef<VertexBuffer> m_transform_buffer;
		TransformVertexData* m_transform_vertex_data = nullptr;

		IntrusiveRef<UniformBufferSet> m_uniform_buffer_set;
		IntrusiveRef<StorageBufferSet> m_storage_buffer_set;

		struct PointLightUB
		{
			uint32_t count{ 0 };
			glm::vec3 padding{};
			PointLight point_lights[1024]{};
		} m_point_lights_ub;

		GPUTimeQueryIndices m_gpu_time_query_indices;

		uint32_t m_viewport_width = 0, m_viewport_height = 0;
		bool m_active = false;
		bool m_needs_resize = false;
		bool m_resources_created = false;

		// flag for flushing scene data on a separate "job" thread
		bool m_use_threads = true;

		SceneRendererData m_scene_data;

		struct DrawCommandData
		{
			IntrusiveRef<MeshData> Mesh;
			uint32_t Submesh_index;
			IntrusiveRef<MaterialTable> Material_table;
			IntrusiveRef<Material> Override_material;

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
}

#endif
