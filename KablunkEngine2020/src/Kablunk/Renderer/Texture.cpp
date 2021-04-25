#include "kablunkpch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Kablunk
{


	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:      KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:    return std::make_shared<OpenGLTexture2D>(path);
		default:						  KB_CORE_ASSERT(false, "Unknown RenderAPI!"); return nullptr;
		}
	}

}