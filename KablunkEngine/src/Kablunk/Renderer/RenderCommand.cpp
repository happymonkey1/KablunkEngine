#include "kablunkpch.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanSwapChain.h" // #TODO temporary while there is no swap chain abstraction

#include "Kablunk/Core/Application.h"

namespace kb::render
{ // start namespace kb::render
u32 rt_get_current_frame_index()
{
	return VulkanContext::Get()->GetSwapchain().GetCurrentBufferIndex();
}

uint32_t get_current_frame_index()
{
	return Application::Get().get_current_frame_index();
}
} // end namespace kb::render
