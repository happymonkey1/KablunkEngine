#include "kablunkpch.h"
#include "Kablunk/Renderer/Texture.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Platform/Vulkan/VulkanTexture.h"

namespace Kablunk
{
	IntrusiveRef<Texture2D> Texture2D::Create(ImageFormat format, uint32_t width, uint32_t height, const void* data)
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::RenderAPI_t::None:    KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!"); return nullptr;
		case RendererAPI::RenderAPI_t::OpenGL:  return IntrusiveRef<OpenGLTexture2D>::Create(format, width, height);
		case RendererAPI::RenderAPI_t::Vulkan:  return IntrusiveRef<VulkanTexture2D>::Create(format, width, height, data);
		default:								KB_CORE_ASSERT(false, "Unknown RenderAPI!"); return nullptr;
		}
	}

	IntrusiveRef<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::RenderAPI_t::None:    KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!"); return nullptr;
		case RendererAPI::RenderAPI_t::OpenGL:  return IntrusiveRef<OpenGLTexture2D>::Create(path);
		case RendererAPI::RenderAPI_t::Vulkan:  return IntrusiveRef<VulkanTexture2D>::Create(path);
		default:								KB_CORE_ASSERT(false, "Unknown RenderAPI!"); return nullptr;
		}
	}

	

}
