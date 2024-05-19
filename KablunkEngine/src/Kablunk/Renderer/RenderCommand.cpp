#include "kablunkpch.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include "Platform/Vulkan/VulkanRendererAPI.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanSwapChain.h" // #TODO temporary while there is no swap chain abstraction

#include "Kablunk/Core/Application.h"

namespace kb::render
{ // start namespace kb::render
u32 rt_get_current_frame_index() noexcept
{
	return VulkanContext::Get()->GetSwapchain().GetCurrentBufferIndex();
}

u32 get_current_frame_index() noexcept
{
	return Application::Get().get_current_frame_index();
}
} // end namespace kb::render
