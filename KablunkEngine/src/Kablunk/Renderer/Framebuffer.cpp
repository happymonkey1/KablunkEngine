#include "kablunkpch.h"

#include "Kablunk/Renderer/Framebuffer.h"
#include "Kablunk/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"

namespace Kablunk
{
	IntrusiveRef<Framebuffer> Framebuffer::Create(const FramebufferSpecification& specs)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::RenderAPI_t::None:
			KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!");
			return nullptr;
		case RendererAPI::RenderAPI_t::OpenGL:
			return IntrusiveRef<OpenGLFramebuffer>::Create(specs);
		case RendererAPI::RenderAPI_t::Vulkan:
			return IntrusiveRef<VulkanFramebuffer>::Create(specs);
		default:
			KB_CORE_ASSERT(false, "Unkown RenderAPI!");
			return nullptr;
		}
	}
}
