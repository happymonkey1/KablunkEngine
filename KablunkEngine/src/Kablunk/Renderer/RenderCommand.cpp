#include "kablunkpch.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Kablunk
{
	// #TODO somehow evaluate at runtime so we can switch between renderers
	Scope<RendererAPI> RenderCommand::s_RendererAPI = CreateScope<VulkanRendererAPI>();

	static RenderCommandQueue s_resource_free_queue[3];

	RenderCommandQueue& RenderCommand::GetRenderResourceReleaseQueue(uint32_t index)
	{
		return s_resource_free_queue[index];
	}

	RenderCommandQueue& RenderCommand::GetRenderCommandQueue()
	{
		return *s_command_queue;
	}
}
