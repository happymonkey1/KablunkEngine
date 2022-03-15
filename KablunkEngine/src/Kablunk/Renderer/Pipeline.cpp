#include "kablunkpch.h"

#include "Kablunk/Renderer/Pipeline.h"

#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanPipeline.h"

namespace Kablunk
{
	IntrusiveRef<Pipeline> Pipeline::Create(const PipelineSpecification& specification)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::RenderAPI_t::OpenGL:		KB_CORE_ASSERT(false, "OpenGL pipeline not implemented!"); return nullptr;
		case RendererAPI::RenderAPI_t::Vulkan:		return IntrusiveRef<VulkanPipeline>::Create(specification);
		default:									KB_CORE_ASSERT(false, "Unknown RenderAPI!"); return nullptr;
		}
	}
}
