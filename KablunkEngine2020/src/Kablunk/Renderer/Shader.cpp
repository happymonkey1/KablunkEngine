#include "kablunkpch.h"
#include "Kablunk/Renderer/Shader.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Kablunk
{
	Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
	{

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			KB_CORE_ASSERT(false, "RendererAPI::None is not supported when creating Shader!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLShader(vertexSrc, fragmentSrc);;
		default:
			KB_CORE_ASSERT(false, "Unkown RenderAPI!");
			return nullptr;
		}
	}
}