#include "kablunkpch.h"
#include "Kablunk/Renderer/RendererAPI.h"

namespace Kablunk
{

	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;
	bool RendererAPI::s_VSYNC = true;
}