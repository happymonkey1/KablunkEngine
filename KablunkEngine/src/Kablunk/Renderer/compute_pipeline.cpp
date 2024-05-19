#include "kablunkpch.h"

#include "Kablunk/Renderer/compute_pipeline.h"
#include "Kablunk/Renderer/RendererAPI.h"
#include "Platform/Vulkan/vulkan_compute_pipeline.h"

namespace kb::render
{ // start namespace kb::render
ref<compute_pipeline> compute_pipeline::create(ref<Shader> p_compute_shader)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    switch (backend)
    {
    case render_backend_type_t::vulkan:
        return static_cast<ref<compute_pipeline>>(ref<vk::compute_pipeline>::Create(p_compute_shader));
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return ref<compute_pipeline>{};
    }
    }
}
} // end namespace kb::render
