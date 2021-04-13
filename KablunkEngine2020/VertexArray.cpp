#include "kablunkpch.h"

#include "VertexArray.h"
#include "Renderer.h"
#include "OpenGLVertexArray.h"

namespace kablunk
{
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::None:
			KB_CORE_FATAL("RendererAPI::NONE is not supported!");
			return nullptr;
		case RendererAPI::OpenGL:
			return new OpenGLVertexArray();
		default:
			KB_CORE_FATAL("Unkown RenderAPI!");
			return nullptr;
		}
	}
}