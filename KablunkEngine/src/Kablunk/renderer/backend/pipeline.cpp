#include "kablunkpch.h"

#include "Kablunk/renderer/backend/pipeline.h"
#include "Kablunk/renderer/Renderer.h"
#include "Kablunk/renderer/backend/vulkan/vulkan_context.h"
#include "kablunk/renderer/backend/vulkan/vulkan_pipeline.h"

namespace kb::render::backend
{ // start namespace kb::render::backend
arc<pipeline> pipeline::create(const pipeline_specification_t& specification)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    if constexpr (backend == render_backend_type_t::vulkan)
    {
        const auto context = Singleton<Renderer>::get().get_graphics_context().as<vk::vulkan_context>();
        return static_cast<arc<pipeline>>(arc<vk::vulkan_pipeline>::Create(context->get_device()->get_vk_device(), specification));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return arc<pipeline>{};
    }
}
} // start namespace kb::render::backend
