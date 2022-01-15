#pragma once
#ifndef KABLUNK_RENDERER_SCENE_RENDERER_H
#define KABLUNK_RENDERER_SCENE_RENDERER_H

#include "Kablunk/Core/Core.h"

#include "Kablunk/Scene/Scene.h"

#include "Kablunk/Renderer/Image.h"
#include "Kablunk/Renderer/RenderPass.h"
#include "Kablunk/Renderer/Pipeline.h"
#include "Kablunk/Renderer/RenderCommandBuffer.h"

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
		SceneRenderer(const IntrusiveRef<Scene>& context, const SceneRendererSpecification& spec);
		~SceneRenderer();

		void Init();
		void SetScene(IntrusiveRef<Scene> context);

		void BeginScene(const SceneRendererCamera& camera);
		void EndScene();

		void SetViewportSize(uint32_t width, uint32_t height);
		IntrusiveRef<RenderPass> GetFinalRenderPass();
		IntrusiveRef<Image2D> GetFinalPassImage();

		void OnImGuiRender();

		static void WaitForThreads();
	private:
		void FlushDrawList();
		void PreRender();
		void ClearPass();
		void GeometryPass();
		void TwoDimensionalPass();
		void CompositePass();

		void ClearPass(IntrusiveRef<RenderPass> render_pass, bool explicit_clear = false);
		
	private:
		IntrusiveRef<Scene> m_context;
		SceneRendererSpecification m_specification;

		IntrusiveRef<RenderCommandBuffer> m_command_buffer;

		IntrusiveRef<Pipeline> m_quad_pipeline;
		IntrusiveRef<Pipeline> m_circle_pipeline;
		IntrusiveRef<Pipeline> m_geometry_pipeline;
		IntrusiveRef<Pipeline> m_composite_pipeline;

		struct GPUTimeQueryIndices
		{
			uint32_t two_dimensional_pass_query;
			uint32_t circle_pass_query;
			uint32_t geometry_pass_query;
			uint32_t composite_pass_query;
		};

		GPUTimeQueryIndices m_gpu_time_query_indices;

		uint32_t m_viewport_width = 0, m_viewport_height = 0;
		bool m_active = false;
		bool m_needs_resize = false;
		bool m_resources_created = false;

		SceneRendererData m_scene_data;

		friend class VulkanRenderer2D;
	};
}

#endif
