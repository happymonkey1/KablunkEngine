#include "kablunkpch.h"

#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Renderer/backend/render_command_buffer.h"

#include "Kablunk/Renderer/Renderer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_render_command_buffer.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

arc<render_command_buffer> render_command_buffer::create(uint32_t count /*= 0*/, const std::string& debug_name /*= ""*/)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    switch (backend)
    {
    case backend::render_backend_type_t::vulkan:
        return static_cast<arc<render_command_buffer>>(arc<backend::vk::vulkan_render_command_buffer>::Create(count, debug_name));
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
    case backend::render_backend_type_t::vulkan:
        return static_cast<arc<render_command_buffer>>(arc<backend::vk::vulkan_render_command_buffer>::Create(debug_name, true));
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return arc<render_command_buffer>{};
    }
    }
}

} // end namespace kb::render::backend
