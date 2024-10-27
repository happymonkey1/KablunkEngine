#include "kablunkpch.h"

#include "Kablunk/renderer/backend/pipeline.h"
#include "Kablunk/renderer/Renderer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_pipeline.h"

namespace kb::render::backend
{ // start namespace kb::render::backend
arc<pipeline> pipeline::create(const pipeline_specification_t& specification)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    if constexpr (backend == backend::render_backend_type_t::vulkan)
    {
        return static_cast<arc<pipeline>>(arc<backend::vk::vulkan_pipeline>::Create(specification));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return arc<pipeline>{};
    }
}
} // start namespace kb::render::backend
