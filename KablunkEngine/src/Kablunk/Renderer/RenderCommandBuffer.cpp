#include "kablunkpch.h"

#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Renderer/RenderCommandBuffer.h"

#include "Platform/Vulkan/VulkanRenderCommandBuffer.h"

namespace kb
{
	ref<RenderCommandBuffer> RenderCommandBuffer::Create(uint32_t count /*= 0*/, const std::string& debug_name /*= ""*/)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::OpenGL:	KB_CORE_ASSERT(false, "OpenGL does not support RenderCommandBuffers!"); return ref<RenderCommandBuffer>{};
		case RendererAPI::render_api_t::Vulkan:	return static_cast<ref<RenderCommandBuffer>>(ref<VulkanRenderCommandBuffer>::Create(count, debug_name));
		default:								KB_CORE_ASSERT(false, "Unknown RendererAPI!"); return ref<RenderCommandBuffer>{};
		}
	}

	ref<RenderCommandBuffer> RenderCommandBuffer::CreateFromSwapChain(const std::string& debug_name /*= ""*/)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::OpenGL:	KB_CORE_ASSERT(false, "OpenGL does not support RenderCommandBuffers!"); return ref<RenderCommandBuffer>{};
        case RendererAPI::render_api_t::Vulkan:	return static_cast<ref<RenderCommandBuffer>>(ref<VulkanRenderCommandBuffer>::Create(debug_name, true));
		default:								KB_CORE_ASSERT(false, "Unknown RendererAPI!"); return ref<RenderCommandBuffer>{};
		}
	}
}
