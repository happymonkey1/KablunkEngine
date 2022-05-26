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

		void Init();
		void SetScene(IntrusiveRef<Scene> context);

		void BeginScene(const SceneRendererCamera& camera);
		void EndScene();

		void SubmitMesh(IntrusiveRef<Mesh> mesh, uint32_t submesh_index, IntrusiveRef<MaterialTable> material_table, const glm::mat4& transform = glm::mat4{ 1.0f }, IntrusiveRef<Material> override_material = nullptr);

		void SetMultiThreaded(bool threaded) { m_use_threads = threaded; }
		bool GetMultiThreaded() const { return m_use_threads; }

		void SetViewportSize(uint32_t width, uint32_t height);
		IntrusiveRef<RenderPass> GetFinalRenderPass();
		IntrusiveRef<RenderPass> GetCompositeRenderPass() { return m_composite_pipeline->GetSpecification().render_pass; }
		IntrusiveRef<RenderPass> GetExternalCompositeRenderPass() { return m_external_composite_render_pass; }
		IntrusiveRef<Image2D> GetFinalPassImage();

		void OnImGuiRender();

		static void WaitForThreads();

	private:
		void FlushDrawList();
		void PreRender();
		void ClearPass();
		void GeometryPass();
		void CompositePass();

		void ClearPass(IntrusiveRef<RenderPass> render_pass, bool explicit_clear = false);
		
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

		bool m_use_threads = false;

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

		friend class VulkanRenderer2D;
	};
}

#endif
