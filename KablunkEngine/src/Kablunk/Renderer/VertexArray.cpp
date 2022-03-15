#include "kablunkpch.h"

#include "Kablunk/Renderer/VertexArray.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Kablunk
{
	IntrusiveRef<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::RenderAPI_t::None:
			KB_CORE_ASSERT(false, "RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::RenderAPI_t::OpenGL:
			return IntrusiveRef<OpenGLVertexArray>::Create();
		default:
			KB_CORE_ASSERT(false, "Unknown RenderAPI!");
			return nullptr;
		}
	}
}
