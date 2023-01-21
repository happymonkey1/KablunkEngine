#include "kablunkpch.h"
#include "Kablunk/Renderer/Buffer.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

#include "Platform/Vulkan/VulkanVertexBuffer.h"
#include "Platform/Vulkan/VulkanIndexBuffer.h"

namespace Kablunk
{
	IntrusiveRef<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::render_api_t::None:	KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!"); return nullptr;
		case RendererAPI::render_api_t::OpenGL:	return IntrusiveRef<OpenGLVertexBuffer>::Create(size);
		case RendererAPI::render_api_t::Vulkan:	return IntrusiveRef<VulkanVertexBuffer>::Create(size);
		default:								KB_CORE_ASSERT(false, "Unkown RenderAPI!"); return nullptr;
		}
	}

	IntrusiveRef<VertexBuffer> VertexBuffer::Create(const void* data, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::render_api_t::None:		KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!"); return nullptr;
		case RendererAPI::render_api_t::OpenGL:		return IntrusiveRef<OpenGLVertexBuffer>::Create(data, size);
		case RendererAPI::render_api_t::Vulkan:		return IntrusiveRef<VulkanVertexBuffer>::Create(data, size);
		default:									KB_CORE_ASSERT(false, "Unkown RenderAPI!"); return nullptr;
		}
	}

	IntrusiveRef<IndexBuffer> IndexBuffer::Create(uint32_t count)
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::render_api_t::None:		KB_CORE_FATAL("RendererAPI::None is not supported!"); return nullptr;
		case RendererAPI::render_api_t::OpenGL:		return IntrusiveRef<OpenGLIndexBuffer>::Create(count);
		case RendererAPI::render_api_t::Vulkan:		return IntrusiveRef<VulkanIndexBuffer>::Create(count);
		default:									KB_CORE_FATAL("Unkown RenderAPI!"); return nullptr;
		}
	}

	IntrusiveRef<IndexBuffer> IndexBuffer::Create(const void* data, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::render_api_t::None:		KB_CORE_FATAL("RendererAPI::None is not supported!"); return nullptr;
		case RendererAPI::render_api_t::OpenGL:		return IntrusiveRef<OpenGLIndexBuffer>::Create(data, count);
		case RendererAPI::render_api_t::Vulkan:		return IntrusiveRef<VulkanIndexBuffer>::Create(data, count);
		default:									KB_CORE_FATAL("Unkown RenderAPI!"); return nullptr;
		}
	}

}
