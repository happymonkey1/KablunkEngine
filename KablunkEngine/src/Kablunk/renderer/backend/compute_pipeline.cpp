#include "kablunkpch.h"

#include "Kablunk/renderer/backend/compute_pipeline.h"
#include "Kablunk/renderer/RendererAPI.h"
#include "kablunk/renderer/backend/vulkan/vulkan_compute_pipeline.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

arc<compute_pipeline> compute_pipeline::create(arc<shader> p_compute_shader)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    switch (backend)
    {
    case render_backend_type_t::vulkan:
        return static_cast<arc<compute_pipeline>>(arc<vk::vulkan_compute_pipeline>::Create(p_compute_shader));
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return arc<compute_pipeline>{};
    }
    }
}

} // end namespace kb::render::backend
