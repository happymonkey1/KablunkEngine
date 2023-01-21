#include "kablunkpch.h"

#include "Kablunk/Renderer/Image.h"

#include "Platform/Vulkan/VulkanImage.h"
#include "Kablunk/Renderer/RendererAPI.h"

namespace Kablunk
{

	IntrusiveRef<Image2D> Image2D::Create(ImageSpecification specification, Buffer buffer)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::OpenGL:	KB_CORE_ASSERT(false, "OpenGL images not supported yet!"); return nullptr;
		case RendererAPI::render_api_t::Vulkan:	return IntrusiveRef<VulkanImage2D>::Create(specification);
		default:								KB_CORE_ASSERT(false, "Unknown RenderAPI!"); return nullptr;	
		}
	}

	IntrusiveRef<Image2D> Image2D::Create(ImageSpecification specification, const void* data /*= nullptr*/)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::OpenGL:	KB_CORE_ASSERT(false, "OpenGL images not supported yet!"); return nullptr;
		case RendererAPI::render_api_t::Vulkan:	return IntrusiveRef<VulkanImage2D>::Create(specification);
		default:								KB_CORE_ASSERT(false, "Unknown RenderAPI!"); return nullptr;
		}
	}

}
