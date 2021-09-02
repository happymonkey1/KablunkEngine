#include "kablunkpch.h"
#include "Kablunk/Renderer/GraphicsContext.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLContext.h"

namespace Kablunk
{
	Scope<GraphicsContext> GraphicsContext::Create(void* window)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::RenderAPI_t::None:
			KB_CORE_ASSERT(false, "RendererAPI::NONE is not supported!");
			return nullptr;
		case RendererAPI::RenderAPI_t::OpenGL:
			return CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
		default:
			KB_CORE_ASSERT(false, "Unknown RenderAPI!");
			return nullptr;
		}
	}
}
