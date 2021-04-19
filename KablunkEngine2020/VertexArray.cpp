#include "kablunkpch.h"

#include "VertexArray.h"
#include "Renderer.h"
#include "OpenGLVertexArray.h"

namespace Kablunk
{
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:
			KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLVertexArray();
		default:
			KB_CORE_ASSERT(false, "Unkown RenderAPI!");
			return nullptr;
		}
	}
}