#pragma once
#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Renderer/RenderCommandQueue.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/Material.h"
#include "Kablunk/Renderer/Pipeline.h"
#include "Kablunk/Renderer/UniformBufferSet.h"
#include "Kablunk/Renderer/StorageBufferSet.h"
#include "Kablunk/Renderer/Mesh.h"
#include "Kablunk/Renderer/MaterialAsset.h"

#include <mutex>

namespace Kablunk::render
{
	// =======
	// new api
	// =======

	// \brief get the os screen position of the viewport within the application
	// used for converting screen to world position when in the editor
	inline const glm::vec2& get_viewport_pos()
	{
		return Singleton<Renderer>::get().get_viewport_pos();
	}

	// \brief get the size of the viewport
	// used for converting screen to world position when in the editor
	inline const glm::vec2& get_viewport_size()
	{
		return Singleton<Renderer>::get().get_viewport_size();
	}
}

namespace Kablunk 
{
	// #TODO refactor to use functions like RenderCommand2D

	class RenderCommand
	{
	public:
		static void Init();

		static void Shutdown()
		{
			KB_CORE_INFO("Renderer shutdown called!");

			s_renderer_api->Shutdown();

			KB_CORE_INFO("Shutting down render command queue!");
			Singleton<RenderCommandQueue>::get().shutdown();

			delete s_renderer_api;
		}


		static void BeginFrame()
		{
			s_renderer_api->BeginFrame();
		}

		static void EndFrame()
		{
			s_renderer_api->EndFrame();
		}

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_renderer_api->SetViewport(x, y, width, height);
		}

		static void SetClearColor(const glm::vec4& color)
		{
			s_renderer_api->SetClearColor(color);
		};

		static void Clear()
		{
			s_renderer_api->Clear();
		};

		static void ClearImage(IntrusiveRef<RenderCommandBuffer> command_buffer, IntrusiveRef<Image2D> image)
		{
			s_renderer_api->ClearImage(command_buffer, image);
		}

		static void DrawIndexed(const IntrusiveRef<VertexArray> vertexArray, uint32_t indexCount = 0)
		{
			s_renderer_api->DrawIndexed(vertexArray, indexCount);
		};

		static void SetWireframeMode(bool draw_wireframe)
		{
			s_renderer_api->SetWireframeMode(draw_wireframe);
		}

		// ======
		// Meshes
		// ======

		static void RenderMesh(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Mesh> mesh, uint32_t submesh_index, IntrusiveRef<MaterialTable> material_table, IntrusiveRef<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count)
		{
			s_renderer_api->RenderMesh(render_command_buffer, pipeline, uniform_buffer_set, storage_buffer_set, mesh, submesh_index, material_table, transform_buffer, transform_offset, instance_count);
		}

		static void RenderMeshWithMaterial(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Mesh> mesh, uint32_t submesh_index, IntrusiveRef<Material> material, IntrusiveRef<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count, Buffer additional_uniforms)
		{
			s_renderer_api->RenderMeshWithMaterial(render_command_buffer, pipeline, uniform_buffer_set, storage_buffer_set, mesh, submesh_index, material, transform_buffer, transform_offset, instance_count, additional_uniforms);
		}

		static void SubmitFullscreenQuad(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<Material> material)
		{
			s_renderer_api->SubmitFullscreenQuad(render_command_buffer, pipeline, uniform_buffer_set, nullptr, material);
		}

		static void SubmitFullscreenQuad(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Material> material)
		{
			s_renderer_api->SubmitFullscreenQuad(render_command_buffer, pipeline, uniform_buffer_set, storage_buffer_set, material);
		}

		static void RenderQuad(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBuffer> storage_buffer_set, IntrusiveRef<Material> material, const glm::mat4& transform)
		{
			s_renderer_api->RenderQuad(render_command_buffer, pipeline, uniform_buffer_set, storage_buffer_set, material, transform);
		}

		static void RenderGeometry(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Material> material, IntrusiveRef<VertexBuffer> vertex_buffer, IntrusiveRef<IndexBuffer> index_buffer, const glm::mat4& transform, uint32_t index_count = 0)
		{
			s_renderer_api->RenderGeometry(render_command_buffer, pipeline, uniform_buffer_set, storage_buffer_set, material, vertex_buffer, index_buffer, transform, index_count);
		}

		static void SetLineWidth(IntrusiveRef<RenderCommandBuffer> render_command_buffer, float line_width)
		{
			s_renderer_api->SetLineWidth(render_command_buffer, line_width);
		}

		static void BeginRenderPass(IntrusiveRef<RenderCommandBuffer> render_command_buffer, const IntrusiveRef<RenderPass>& render_pass, bool explicit_clear = false)
		{
			s_renderer_api->BeginRenderPass(render_command_buffer, render_pass, explicit_clear);
		}

		static void EndRenderPass(IntrusiveRef<RenderCommandBuffer> render_command_buffer)
		{
			s_renderer_api->EndRenderPass(render_command_buffer);
		}

		static void WaitAndRender()
		{
			s_renderer_api->WaitAndRender();
		}

		static RendererAPI* GetRenderer()
		{
			return s_renderer_api;
		}

		// #TODO this is vulkan only so we should figure out an api agnostic way of dealing with this
		template <typename FuncT>
		static void Submit(FuncT&& func)
		{
			std::lock_guard lock{ s_submit_mutex };
			auto render_cmd = [](void* ptr)
			{
				auto p_func = (FuncT*)ptr;
				(*p_func)();

				p_func->~FuncT();
			};

			auto storage_buffer = GetRenderCommandQueue().Allocate(render_cmd, sizeof(func));
			new (storage_buffer) FuncT(std::forward<FuncT>(func));
		}

		template <typename FuncT>
		static void SubmitResourceFree(FuncT&& func)
		{
			auto render_cmd = [](void* ptr)
			{
				auto p_func = (FuncT*)ptr;
				(*p_func)();

				p_func->~FuncT();
			};

			Submit([render_cmd, func]()
				{
					const uint32_t index = Renderer::GetCurrentFrameIndex();
					auto storage_buffer = GetRenderResourceReleaseQueue(index).Allocate(render_cmd, sizeof(func));
					new (storage_buffer) FuncT(std::forward<FuncT>((FuncT&&)func));
				});
		}

		static RenderCommandQueue& GetRenderResourceReleaseQueue(uint32_t index);
		// #TODO this is vulkan only so we should figure out an api agnostic way of dealing with this
		static RenderCommandQueue& GetRenderCommandQueue();
	private:
		inline static RendererAPI* s_renderer_api;
		inline static std::mutex s_submit_mutex;
	};
}
