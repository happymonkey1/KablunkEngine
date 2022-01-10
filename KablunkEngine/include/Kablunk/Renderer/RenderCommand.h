#pragma once
#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Renderer/RenderCommandQueue.h"

namespace Kablunk 
{
	class RenderCommand
	{
	public:
		static void Init() 
		{ 
			// #TODO fix the renderapi because this is just horrible
			if (s_RendererAPI->GetAPI() != RendererAPI::RenderAPI_t::OpenGL)
				s_command_queue = new RenderCommandQueue();

			s_RendererAPI->Init();
		}

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		};

		static void Clear()
		{
			s_RendererAPI->Clear();
		};

		static void DrawIndexed(const IntrusiveRef<VertexArray> vertexArray, uint32_t indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		};

		static void SetWireframeMode(bool draw_wireframe)
		{
			s_RendererAPI->SetWireframeMode(draw_wireframe);
		}

		static void WaitAndRender()
		{
			// #TODO fix the renderapi because this is just horrible
			if (s_RendererAPI->GetAPI() != RendererAPI::RenderAPI_t::OpenGL)
				s_command_queue->Execute();
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
	private:
		// #TODO this is vulkan only so we should figure out an api agnostic way of dealing with this
		static RenderCommandQueue& GetRenderCommandQueue();
	private:
		static Scope<RendererAPI> s_RendererAPI;

		// #TODO fix the renderapi because this is just horrible
		inline static RenderCommandQueue* s_command_queue = nullptr;
	};
}
