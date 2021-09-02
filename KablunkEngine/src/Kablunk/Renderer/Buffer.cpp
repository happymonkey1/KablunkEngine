#include "kablunkpch.h"
#include "Kablunk/Renderer/Buffer.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Kablunk
{
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::RenderAPI_t::None:
			KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!");
			return nullptr;
		case RendererAPI::RenderAPI_t::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(size);
		default:
			KB_CORE_ASSERT(false, "Unkown RenderAPI!");
			return nullptr;
		}
	}

	Kablunk::Ref<Kablunk::VertexBuffer> VertexBuffer::Create(const void* data, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::RenderAPI_t::None:
			KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!");
			return nullptr;
		case RendererAPI::RenderAPI_t::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(data, size);
		default:
			KB_CORE_ASSERT(false, "Unkown RenderAPI!");
			return nullptr;
		}
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t count)
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::RenderAPI_t::None:
			KB_CORE_FATAL("RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::RenderAPI_t::OpenGL:
			return CreateRef<OpenGLIndexBuffer>(count);
		default:
			KB_CORE_FATAL("Unkown RenderAPI!");
			return nullptr;
		}
	}

	Kablunk::Ref<Kablunk::IndexBuffer> IndexBuffer::Create(const void* data, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::RenderAPI_t::None:
			KB_CORE_FATAL("RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::RenderAPI_t::OpenGL:
			return CreateRef<OpenGLIndexBuffer>(data, count);
		default:
			KB_CORE_FATAL("Unkown RenderAPI!");
			return nullptr;
		}
	}

}
