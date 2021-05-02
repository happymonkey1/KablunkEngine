#include "kablunkpch.h"
#include "Kablunk/Renderer/Buffer.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Kablunk
{
	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(vertices, size);
		default:
			KB_CORE_ASSERT(false, "Unkown RenderAPI!");
			return nullptr;
		}
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:
			KB_CORE_FATAL("RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLIndexBuffer>(indices, count);
		default:
			KB_CORE_FATAL("Unkown RenderAPI!");
			return nullptr;
		}
	}
}