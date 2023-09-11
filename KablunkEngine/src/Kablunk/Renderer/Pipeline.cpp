#include "kablunkpch.h"

#include "Kablunk/Renderer/Pipeline.h"

#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanPipeline.h"

namespace Kablunk
{
	ref<Pipeline> Pipeline::Create(const PipelineSpecification& specification)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::OpenGL:		KB_CORE_ASSERT(false, "OpenGL pipeline not implemented!"); return nullptr;
		case RendererAPI::render_api_t::Vulkan:		return ref<VulkanPipeline>::Create(specification);
		default:									KB_CORE_ASSERT(false, "Unknown RenderAPI!"); return nullptr;
		}
	}
}
