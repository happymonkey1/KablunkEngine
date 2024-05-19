#include "kablunkpch.h"

#include "Kablunk/Renderer/render_pass.h"

#include "Kablunk/Renderer/Renderer.h"

#include "Platform/Vulkan/vulkan_render_pass.h"

namespace kb::render
{ // start namespace kb::render

ref<render_pass> render_pass::create(const render_pass_specification& specification) noexcept
{
    constexpr auto backend = Renderer::get_render_backend_type();
    if constexpr (backend == render_backend_type_t::vulkan)
    {
        return static_cast<ref<render_pass>>(ref<vulkan_render_pass>::Create(specification));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render_backend_type!");
        return ref<render_pass>{};
    }
}

} // end namespace kb::render
