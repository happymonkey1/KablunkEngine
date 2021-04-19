#include "kablunkpch.h"
#include "RenderCommand.h"

#include "OpenGLRendererAPI.h"

namespace Kablunk
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}