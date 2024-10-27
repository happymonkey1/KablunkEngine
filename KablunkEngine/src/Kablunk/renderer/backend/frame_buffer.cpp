#include "kablunkpch.h"

#include "Kablunk/renderer/backend/frame_buffer.h"
#include "Kablunk/renderer/Renderer.h"

#include "kablunk/renderer/backend/vulkan/vulkan_frame_buffer.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

arc<frame_buffer> frame_buffer::create(const frame_buffer_specification& specs)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    arc<frame_buffer> frame_buffer{};
    switch (backend)
    {
    case render_backend_type_t::vulkan:
        frame_buffer = static_cast<arc<backend::frame_buffer>>(arc<vk::vulkan_frame_buffer>::Create(specs));
        break;
    default:
    {
        KB_CORE_ASSERT(
            false,
            "[Framebuffer::Create]: Unhandled render backend {}!",
            static_cast<std::underlying_type_t<render_backend_type_t>>(backend)
        );
        return arc<backend::frame_buffer>{};
    }
    }

	return frame_buffer;
}

} // end namespace kb::render::backend
