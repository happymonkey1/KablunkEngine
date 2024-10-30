#include "kablunkpch.h"

#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Renderer/backend/render_command_buffer.h"

#include "Kablunk/Renderer/Renderer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_context.h"
#include "kablunk/renderer/backend/vulkan/vulkan_render_command_buffer.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

arc<render_command_buffer> render_command_buffer::create(uint32_t count /*= 0*/, const std::string& debug_name /*= ""*/)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    switch (backend)
    {
    case render_backend_type_t::vulkan:
    {
        const auto context = Singleton<Renderer>::get()
            .get_graphics_context()
            .as<vk::vulkan_context>();
        const auto device = context->get_device();
        return static_cast<arc<render_command_buffer>>(arc<vk::vulkan_render_command_buffer>::Create(
            device,
            count,
            debug_name
        ));
    }
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return arc<render_command_buffer>{};
    }
    }
}

arc<render_command_buffer> render_command_buffer::create_from_swap_chain(const std::string& debug_name /*= ""*/)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    switch (backend)
    {
    case render_backend_type_t::vulkan:
    {
        const auto context = Singleton<Renderer>::get()
            .get_graphics_context()
            .as<vk::vulkan_context>();
        const auto device = context->get_device();
        return static_cast<arc<render_command_buffer>>(arc<vk::vulkan_render_command_buffer>::Create(
            device,
            debug_name,
            true
        ));
    }
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return arc<render_command_buffer>{};
    }
    }
}

} // end namespace kb::render::backend
