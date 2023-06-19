#include "kablunkpch.h"

#include "Kablunk/Renderer/compute_pipeline.h"
#include "Kablunk/Renderer/RendererAPI.h"
#include "Platform/Vulkan/vulkan_compute_pipeline.h"

namespace kb::render
{ // start namespace kb::render



Kablunk::ref<kb::render::compute_pipeline> compute_pipeline::create(Kablunk::ref<Kablunk::Shader> p_compute_shader)
{
    switch (Kablunk::RendererAPI::GetAPI())
    {
        case Kablunk::RendererAPI::render_api_t::OpenGL:
        {
            KB_ASSERT(false, "[compute_pipeline]: compute pipelines are not supported on OpenGL!");
            return nullptr;
        }
        case Kablunk::RendererAPI::render_api_t::Vulkan:
        {
            return Kablunk::ref<kb::vk::compute_pipeline>::Create(p_compute_shader);
        }
        default:
        {
            KB_ASSERT(false, "[compute_pipeline]: unhandled rendererAPI!");
            return nullptr;
        }
    }
}

} // end namespace kb::render
