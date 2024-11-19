#include "kablunkpch.h"

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
        const u32 graphics_family_queue_index = device->get_physical_device()
            ->get_queue_family_indices()
            .Graphics_family
            .value();
        return static_cast<arc<render_command_buffer>>(arc<vk::vulkan_render_command_buffer>::Create(
            device->get_vk_device(),
            graphics_family_queue_index,
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
        auto context = Singleton<Renderer>::get()
            .get_graphics_context()
            .as<vk::vulkan_context>();
        const auto device = context->get_device();
        auto vulkan_swap_chain = weak_ptr{ context->get_vulkan_swap_chain() };
        return static_cast<arc<render_command_buffer>>(arc<vk::vulkan_render_command_buffer>::Create(
            device->get_vk_device(),
            vulkan_swap_chain,
            debug_name
        ));
    }
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return arc<render_command_buffer>{};
    }
    }
}

} // end namespace kb::render::backend
