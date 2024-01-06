#include "kablunkpch.h"
#include "Kablunk/Renderer/GraphicsContext.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLContext.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace kb
{
	ref<GraphicsContext> GraphicsContext::Create(void* window)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::render_api_t::None:
			KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!");
			return ref<GraphicsContext>{};
		case RendererAPI::render_api_t::OpenGL:
			return static_cast<ref<GraphicsContext>>(ref<OpenGLContext>::Create(static_cast<GLFWwindow*>(window)));
		case RendererAPI::render_api_t::Vulkan:
			return static_cast<ref<GraphicsContext>>(ref<VulkanContext>::Create(static_cast<GLFWwindow*>(window)));
		default:
			KB_CORE_ASSERT(false, "Unknown RenderAPI!");
			return ref<GraphicsContext>{};
		}
	}
}
