#include "kablunkpch.h"

#include "Kablunk/renderer/backend/frame_buffer.h"
#include "Kablunk/renderer/Renderer.h"

#include "kablunk/renderer/backend/vulkan/vulkan_frame_buffer.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

arc<frame_buffer> frame_buffer::create(const frame_buffer_specification_t& specs)
{
    switch (const auto backend = Singleton<Renderer>::get().get_render_backend_type())
    {
    case render_backend_type_t::vulkan:
    {
        const auto context = Singleton<Renderer>::get().get_graphics_context().as<vk::vulkan_context>();
        return static_cast<arc<frame_buffer>>(arc<vk::vulkan_frame_buffer>::Create(
            context->get_device()->get_vk_device(),
            specs
        ));
    }
    case render_backend_type_t::none:
        return arc<frame_buffer>{};
    default:
    {
        KB_CORE_ASSERT(
            false,
            "[Framebuffer::Create]: Unhandled render backend {}!",
            static_cast<std::underlying_type_t<render_backend_type_t>>(backend)
        );
        return arc<frame_buffer>{};
    }
    }
}

} // end namespace kb::render::backend
