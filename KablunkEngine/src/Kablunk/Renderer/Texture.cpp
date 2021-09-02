#include "kablunkpch.h"
#include "Kablunk/Renderer/Texture.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Kablunk
{
	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::RenderAPI_t::None:      KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!"); return nullptr;
		case RendererAPI::RenderAPI_t::OpenGL:    return CreateRef<OpenGLTexture2D>(width, height);
		default:						  KB_CORE_ASSERT(false, "Unknown RenderAPI!"); return nullptr;
		}
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::RenderAPI_t::None:      KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!"); return nullptr;
		case RendererAPI::RenderAPI_t::OpenGL:    return CreateRef<OpenGLTexture2D>(path);
		default:						  KB_CORE_ASSERT(false, "Unknown RenderAPI!"); return nullptr;
		}
	}

	

}
