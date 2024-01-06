#include "kablunkpch.h"

#include "Kablunk/Renderer/RenderPass.h"

#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanRenderPass.h"

namespace kb
{
	ref<RenderPass> RenderPass::Create(const RenderPassSpecification& specification)
	{
		
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::OpenGL:		KB_CORE_ASSERT(false, "OpenGL RenderPass not implemented!"); return ref<RenderPass>{};
		case RendererAPI::render_api_t::Vulkan:		return static_cast<ref<RenderPass>>(ref<VulkanRenderPass>::Create(specification));
		default:									KB_CORE_ASSERT(false, "Unknown RenderAPI!"); return ref<RenderPass>{};
		}
	}
}
