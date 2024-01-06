#include "kablunkpch.h"

#include "Kablunk/Renderer/VertexArray.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace kb
{
	ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::render_api_t::None:
			KB_CORE_ASSERT(false, "RendererAPI::None is not supported!");
			return ref<VertexArray>{};
		case RendererAPI::render_api_t::OpenGL:
			return static_cast<ref<VertexArray>>(ref<OpenGLVertexArray>::Create());
		default:
			KB_CORE_ASSERT(false, "Unknown RenderAPI!");
			return ref<VertexArray>{};
		}
	}
}
