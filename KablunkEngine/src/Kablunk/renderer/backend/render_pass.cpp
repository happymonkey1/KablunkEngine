#include "kablunkpch.h"

#include "Kablunk/renderer/backend/render_pass.h"
#include "Kablunk/renderer/Renderer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_render_pass.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

arc<render_pass> render_pass::create(const render_pass_specification& specification) noexcept
{
    switch (Singleton<Renderer>::get().get_render_backend_type())
    {
    case render_backend_type_t::vulkan:
        return static_cast<arc<render_pass>>(arc<vk::vulkan_render_pass>::Create(specification));
    case render_backend_type_t::none:
        return arc<render_pass>{};
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render_backend_type!");
        return arc<render_pass>{};
    }
    }
}

} // end namespace kb::render::backend
