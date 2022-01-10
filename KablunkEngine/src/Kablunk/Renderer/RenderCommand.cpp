#include "kablunkpch.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Kablunk
{
	// #TODO somehow evaluate at runtime so we can switch between renderers
	Scope<RendererAPI> RenderCommand::s_RendererAPI = CreateScope<VulkanRendererAPI>();

	RenderCommandQueue& RenderCommand::GetRenderCommandQueue()
	{
		return *s_command_queue;
	}
}
