#include "kablunkpch.h"
#include "Kablunk/Renderer/Texture.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanTexture.h"

namespace kb
{
	ref<Texture2D> Texture2D::Create(ImageFormat format, uint32_t width, uint32_t height, const void* data)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::render_api_t::None:    KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!"); return ref<Texture2D>{};
		case RendererAPI::render_api_t::Vulkan:  return static_cast<ref<Texture2D>>(ref<VulkanTexture2D>::Create(format, width, height, data));
		default:								KB_CORE_ASSERT(false, "Unknown RenderAPI!"); return ref<Texture2D>{};
		}
	}

	ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
        {
        case RendererAPI::render_api_t::None:    KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!"); return ref<Texture2D>{};
		case RendererAPI::render_api_t::Vulkan:  return static_cast<ref<Texture2D>>(ref<VulkanTexture2D>::Create(path));
		default:								KB_CORE_ASSERT(false, "Unknown RenderAPI!"); return ref<Texture2D>{};
		}
	}
}
