#include "kablunkpch.h"

#include "Kablunk/renderer/backend/graphics_context.h"
#include "Kablunk/renderer/Renderer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_context.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

arc<graphics_context> graphics_context::create(void* window)
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    switch (backend)
    {
    case render_backend_type_t::vulkan:
        return static_cast<arc<graphics_context>>(arc<vk::vulkan_context>::Create(static_cast<GLFWwindow*>(window)));
    default:
    {
        KB_CORE_ASSERT(
            false,
            "[GraphicsContext::Create]: Unhandled render backend {}",
            static_cast<std::underlying_type_t<render_backend_type_t>>(backend)
        );
        return arc<graphics_context>{};
    }
    }
}

} // end namespace kb::render::backend
