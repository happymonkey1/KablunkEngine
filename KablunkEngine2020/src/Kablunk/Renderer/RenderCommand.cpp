#include "kablunkpch.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Kablunk
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}