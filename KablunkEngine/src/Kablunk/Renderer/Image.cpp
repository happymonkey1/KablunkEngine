#include "kablunkpch.h"

#include "Kablunk/Renderer/Image.h"

#include "Platform/Vulkan/VulkanImage.h"
#include "Kablunk/Renderer/RendererAPI.h"

namespace kb
{
	ref<Image2D> Image2D::Create(ImageSpecification specification, owning_buffer buffer)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::Vulkan:	return static_cast<ref<Image2D>>(ref<VulkanImage2D>::Create(specification));
		default:								KB_CORE_ASSERT(false, "Unknown RenderAPI!"); return ref<Image2D>{};
		}
	}

	ref<Image2D> Image2D::Create(ImageSpecification specification, const void* data /*= nullptr*/)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::Vulkan:	return static_cast<ref<Image2D>>(ref<VulkanImage2D>::Create(specification));
		default:								KB_CORE_ASSERT(false, "Unknown RenderAPI!"); return ref<Image2D>{};
		}
	}
}
