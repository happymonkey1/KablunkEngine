#include "kablunkpch.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Kablunk::render
{

	static RenderCommandQueue s_resource_free_queue[3];

	RenderCommandQueue& get_render_resource_release_queue(uint32_t index)
	{
		return s_resource_free_queue[index];
	}

	RenderCommandQueue& get_render_command_queue()
	{
		return Singleton<RenderCommandQueue>::get();
	}

}
