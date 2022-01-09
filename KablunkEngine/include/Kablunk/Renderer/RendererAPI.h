#pragma once

#include "Kablunk/Renderer/VertexArray.h"
#include <glm/glm.hpp>

namespace Kablunk 
{
	class RendererAPI
	{
	public:
		enum class RenderAPI_t
		{
			None = 0, OpenGL = 1, Vulkan = 2
		};

	public:
		virtual ~RendererAPI() = default;
		
		virtual void Init() = 0;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void SetWireframeMode(bool draw_wireframe) = 0;

		static inline RenderAPI_t GetAPI() { return s_API; };
	private:
		inline static RenderAPI_t s_API = RenderAPI_t::OpenGL;
	};

}
