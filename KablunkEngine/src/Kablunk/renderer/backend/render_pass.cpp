#include "kablunkpch.h"

#include "Kablunk/renderer/backend/render_pass.h"
#include "Kablunk/renderer/Renderer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_render_pass.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

arc<render_pass> render_pass::create(const render_pass_specification& specification) noexcept
{
    constexpr auto backend = Renderer::get_render_backend_type();
    if constexpr (backend == backend::render_backend_type_t::vulkan)
    {
        return static_cast<arc<render_pass>>(arc<backend::vk::vulkan_render_pass>::Create(specification));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render_backend_type!");
        return arc<render_pass>{};
    }
}

} // end namespace kb::render::backend
