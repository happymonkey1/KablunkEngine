#include "kablunkpch.h"

#include "Kablunk/Renderer/Framebuffer.h"
#include "Kablunk/Renderer/Renderer.h"

#include "Platform/Vulkan/VulkanFramebuffer.h"

namespace kb
{
	ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& specs)
	{
		ref<Framebuffer> framebuffer;
		switch (Renderer::GetAPI())
		{
		case RendererAPI::render_api_t::None:	KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!"); return ref<Framebuffer>{};
		case RendererAPI::render_api_t::Vulkan:	framebuffer = ref<VulkanFramebuffer>::Create(specs); break;
		default:								KB_CORE_ASSERT(false, "Unkown RenderAPI!"); return ref<Framebuffer>{};
		}

		FramebufferPool::Get()->Add(framebuffer);
		return framebuffer;
	}

	FramebufferPool* FramebufferPool::s_instance = new FramebufferPool{};

	FramebufferPool::FramebufferPool(uint32_t max_framebuffers /*= 32*/)
	{

	}

	FramebufferPool::~FramebufferPool() = default;

    std::weak_ptr<kb::Framebuffer> FramebufferPool::AllocateBuffer()
	{
		return {};
	}

	void FramebufferPool::Add(const ref<Framebuffer>& framebuffer)
	{
		m_pool.push_back(framebuffer);
	}

}
