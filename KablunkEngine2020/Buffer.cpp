#include "kablunkpch.h"
#include "Buffer.h"

#include "Renderer.h"
#include "OpenGLBuffer.h"

namespace Kablunk
{
	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:
			KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLVertexBuffer(vertices, size);
		default:
			KB_CORE_ASSERT(false, "Unkown RenderAPI!");
			return nullptr;
		}
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:
			KB_CORE_FATAL("RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLIndexBuffer(indices, count);
		default:
			KB_CORE_FATAL("Unkown RenderAPI!");
			return nullptr;
		}
	}
}