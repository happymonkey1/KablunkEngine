#include "kablunkpch.h"

#include "Kablunk/Renderer/UniformBufferSet.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanUniformBufferSet.h"

namespace kb
{ // start namespace kb
arc<UniformBufferSet> UniformBufferSet::create(u32 p_size, u32 p_frames_in_flight)
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    switch (backend)
    {
    case render::render_backend_type_t::vulkan:
        return static_cast<arc<UniformBufferSet>>(arc<VulkanUniformBufferSet>::Create(p_size, p_frames_in_flight));
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return arc<UniformBufferSet>{};
    }
    }
}
} // end namespace kb
