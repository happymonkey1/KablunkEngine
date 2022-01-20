#pragma once
#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Renderer/RenderCommandQueue.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/Material.h"
#include "Kablunk/Renderer/Pipeline.h"
#include "Kablunk/Renderer/UniformBufferSet.h"
#include "Kablunk/Renderer/StorageBufferSet.h"

namespace Kablunk 
{
	class RenderCommand
	{
	public:
		static void Init();
		static void Shutdown()
		{
			s_renderer_api->Shutdown();

			delete s_command_queue;
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

		static void SubmitFullscreenQuad(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<Material> material)
		{
			s_renderer_api->SubmitFullscreenQuad(render_command_buffer, pipeline, uniform_buffer_set, nullptr, material);
		}

		static void SubmitFullscreenQuad(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Material> material)
		{
			s_renderer_api->SubmitFullscreenQuad(render_command_buffer, pipeline, uniform_buffer_set, storage_buffer_set, material);
		}

		static void RenderGeometry(IntrusiveRef<RenderCommandBuffer> render_command_buffer, IntrusiveRef<Pipeline> pipeline, IntrusiveRef<UniformBufferSet> uniform_buffer_set, IntrusiveRef<StorageBufferSet> storage_buffer_set, IntrusiveRef<Material> material, IntrusiveRef<VertexBuffer> vertex_buffer, IntrusiveRef<IndexBuffer> index_buffer, const glm::mat4& transform, uint32_t index_count = 0)
		{
			s_renderer_api->RenderGeometry(render_command_buffer, pipeline, uniform_buffer_set, storage_buffer_set, material, vertex_buffer, index_buffer, transform, index_count);
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
	private:
		inline static RendererAPI* s_renderer_api;

		// #TODO fix the renderapi because this is just horrible
		inline static RenderCommandQueue* s_command_queue = nullptr;
	};
}
