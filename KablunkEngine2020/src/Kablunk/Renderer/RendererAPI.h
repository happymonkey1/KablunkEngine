#pragma once

#include "Kablunk/Renderer/VertexArray.h"
#include <glm/glm.hpp>

namespace Kablunk 
{
	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1
		};

	public:
		
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) = 0;

		static inline API GetAPI() { return s_API; };
		static inline bool GetVsync() { return s_VSYNC; };
	private:
		static API s_API;
		static bool s_VSYNC;
	};

}
