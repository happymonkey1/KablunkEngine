#include "kablunkpch.h"

#include "Kablunk/Renderer/RenderPass.h"

#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanRenderPass.h"

namespace Kablunk
{
	IntrusiveRef<RenderPass> RenderPass::Create(const RenderPassSpecification& specification)
	{
		
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::RenderAPI_t::OpenGL:		KB_CORE_ASSERT(false, "OpenGL RenderPass not implemented!"); return nullptr;
		case RendererAPI::RenderAPI_t::Vulkan:		return IntrusiveRef<VulkanRenderPass>::Create(specification);
		default:									KB_CORE_ASSERT(false, "Unknown RenderAPI!"); return nullptr;
		}
	}
}
