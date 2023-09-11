#include "kablunkpch.h"

#include "Kablunk/Renderer/Framebuffer.h"
#include "Kablunk/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"

namespace Kablunk
{
	ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& specs)
	{
		ref<Framebuffer> framebuffer;
		switch (Renderer::GetAPI())
		{
		case RendererAPI::render_api_t::None:	KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!"); return nullptr;
		case RendererAPI::render_api_t::OpenGL:	framebuffer = ref<OpenGLFramebuffer>::Create(specs); break;
		case RendererAPI::render_api_t::Vulkan:	framebuffer = ref<VulkanFramebuffer>::Create(specs); break;
		default:								KB_CORE_ASSERT(false, "Unkown RenderAPI!"); return nullptr;
		}

		FramebufferPool::Get()->Add(framebuffer);
		return framebuffer;
	}

	FramebufferPool* FramebufferPool::s_instance = new FramebufferPool{};

	FramebufferPool::FramebufferPool(uint32_t max_framebuffers /*= 32*/)
	{

	}

	FramebufferPool::~FramebufferPool()
	{

	}

	std::weak_ptr<Kablunk::Framebuffer> FramebufferPool::AllocateBuffer()
	{
		return std::weak_ptr<Framebuffer>();
	}

	void FramebufferPool::Add(const ref<Framebuffer>& framebuffer)
	{
		m_pool.push_back(framebuffer);
	}

}
