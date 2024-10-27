#include "kablunkpch.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include "kablunk/renderer/backend/vulkan/vulkan_context.h"
#include "kablunk/renderer/backend/vulkan/vulkan_swap_chain.h" // #TODO temporary while there is no swap chain abstraction

#include "Kablunk/Core/Application.h"

namespace kb::render
{ // start namespace kb::render

u32 rt_get_current_frame_index() noexcept
{
	return backend::vk::vulkan_context::get()->get_vulkan_swap_chain()->get_current_buffer_index();
}

u32 get_current_frame_index() noexcept
{
	return Application::Get().get_current_frame_index();
}

} // end namespace kb::render
