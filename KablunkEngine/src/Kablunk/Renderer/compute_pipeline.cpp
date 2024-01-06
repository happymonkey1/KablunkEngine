#include "kablunkpch.h"

#include "Kablunk/Renderer/compute_pipeline.h"
#include "Kablunk/Renderer/RendererAPI.h"
#include "Platform/Vulkan/vulkan_compute_pipeline.h"

namespace kb::render
{ // start namespace kb::render
ref<compute_pipeline> compute_pipeline::create(ref<Shader> p_compute_shader)
{
    switch (RendererAPI::GetAPI())
    {
        case RendererAPI::render_api_t::OpenGL:
        {
            KB_ASSERT(false, "[compute_pipeline]: compute pipelines are not supported on OpenGL!");
            return ref<compute_pipeline>{};
        }
        case RendererAPI::render_api_t::Vulkan:
        {
            return static_cast<ref<compute_pipeline>>(ref<vk::compute_pipeline>::Create(p_compute_shader));
        }
        default:
        {
            KB_ASSERT(false, "[compute_pipeline]: unhandled rendererAPI!");
            return ref<compute_pipeline>{};
        }
    }
}
} // end namespace kb::render
