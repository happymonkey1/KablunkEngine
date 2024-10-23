#include "kablunkpch.h"

#include "Kablunk/Renderer/render_pass.h"

#include "Kablunk/Renderer/Renderer.h"

#include "Platform/Vulkan/vulkan_render_pass.h"

namespace kb::render
{ // start namespace kb::render

arc<render_pass> render_pass::create(const render_pass_specification& specification) noexcept
{
    constexpr auto backend = Renderer::get_render_backend_type();
    if constexpr (backend == render_backend_type_t::vulkan)
    {
        return static_cast<arc<render_pass>>(arc<vulkan_render_pass>::Create(specification));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render_backend_type!");
        return arc<render_pass>{};
    }
}

} // end namespace kb::render
