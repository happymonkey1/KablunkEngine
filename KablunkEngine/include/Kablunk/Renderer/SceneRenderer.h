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

namespace Kablunk
{
	struct SceneRendererSpecification
	{
		bool swap_chain_target = false;
	};

	struct SceneRendererCamera
	{
		Camera camera;
		glm::mat4 view_mat;
	};

	struct SceneRendererData
	{
		SceneRendererCamera camera;
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

		GPUTimeQueryIndices m_gpu_time_query_indices;

		uint32_t m_viewport_width = 0, m_viewport_height = 0;
		bool m_active = false;
		bool m_needs_resize = false;
		bool m_resources_created = false;

		bool m_use_threads = false;

		SceneRendererData m_scene_data;

		friend class VulkanRenderer2D;
	};
}

#endif
