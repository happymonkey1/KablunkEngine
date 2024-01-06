#include "kablunkpch.h"
#include "Kablunk/Renderer/Buffer.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

#include "Platform/Vulkan/VulkanVertexBuffer.h"
#include "Platform/Vulkan/VulkanIndexBuffer.h"

namespace kb
{
	ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::render_api_t::None:	KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!"); return ref<VertexBuffer>{};
		case RendererAPI::render_api_t::OpenGL:	return static_cast<ref<VertexBuffer>>(ref<OpenGLVertexBuffer>::Create(size));
		case RendererAPI::render_api_t::Vulkan:	return static_cast<ref<VertexBuffer>>(ref<VulkanVertexBuffer>::Create(size));
		default:								KB_CORE_ASSERT(false, "Unkown RenderAPI!"); return ref<VertexBuffer>{};
		}
	}

	ref<VertexBuffer> VertexBuffer::Create(const void* data, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::render_api_t::None:		KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!"); return ref<VertexBuffer>{};
		case RendererAPI::render_api_t::OpenGL:		return static_cast<ref<VertexBuffer>>(ref<OpenGLVertexBuffer>::Create(data, size));
		case RendererAPI::render_api_t::Vulkan:		return static_cast<ref<VertexBuffer>>(ref<VulkanVertexBuffer>::Create(data, size));
		default:									KB_CORE_ASSERT(false, "Unkown RenderAPI!"); return ref<VertexBuffer>{};
		}
	}

	ref<IndexBuffer> IndexBuffer::Create(uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::render_api_t::None:		KB_CORE_FATAL("RendererAPI::None is not supported!"); return ref<IndexBuffer>{};
		case RendererAPI::render_api_t::OpenGL:		return static_cast<ref<IndexBuffer>>(ref<OpenGLIndexBuffer>::Create(count));
		case RendererAPI::render_api_t::Vulkan:		return static_cast<ref<IndexBuffer>>(ref<VulkanIndexBuffer>::Create(count));
		default:									KB_CORE_FATAL("Unkown RenderAPI!"); return ref<IndexBuffer>{};
		}
	}

	ref<IndexBuffer> IndexBuffer::Create(const void* data, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::render_api_t::None:		KB_CORE_FATAL("RendererAPI::None is not supported!"); return ref<IndexBuffer>{};
        case RendererAPI::render_api_t::OpenGL:		return static_cast<ref<IndexBuffer>>(ref<OpenGLIndexBuffer>::Create(data, count));
        case RendererAPI::render_api_t::Vulkan:		return static_cast<ref<IndexBuffer>>(ref<VulkanIndexBuffer>::Create(data, count));
		default:									KB_CORE_FATAL("Unkown RenderAPI!"); return ref<IndexBuffer>{};
		}
	}

}
