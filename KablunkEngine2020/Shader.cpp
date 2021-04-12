#include "kablunkpch.h"
#include "Shader.h"

#include "Renderer.h"
#include "OpenGLShader.h"

namespace kablunk
{
	Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			KB_CORE_FATAL("RendererAPI::None is not supported when creating Shader!");
			return nullptr;
		case RendererAPI::OpenGL:
			return new OpenGLShader(vertexSrc, fragmentSrc);;
		default:
			KB_CORE_FATAL("Unkown RenderAPI!");
			return nullptr;
		}
	}
}