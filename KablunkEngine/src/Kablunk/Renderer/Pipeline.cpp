#include "kablunkpch.h"

#include "Kablunk/Renderer/Pipeline.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanPipeline.h"

namespace kb::render
{ // start namespace kb::render
ref<Pipeline> Pipeline::Create(const PipelineSpecification& specification)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    if constexpr (backend == render_backend_type_t::vulkan)
    {
        return static_cast<ref<Pipeline>>(ref<VulkanPipeline>::Create(specification));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return ref<Pipeline>{};
    }
}
} // start namespace kb::render
