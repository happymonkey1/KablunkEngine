#include "kablunkpch.h"

#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Renderer/uniform_buffer.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/render_backend.h"
#include "Platform/Vulkan/vulkan_uniform_buffer.h"

namespace kb
{ // start namespace kb
arc<uniform_buffer> uniform_buffer::create(u32 p_size)
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    if constexpr (backend == render::render_backend_type_t::vulkan)
    {
        return static_cast<arc<uniform_buffer>>(arc<vulkan_uniform_buffer>::Create(p_size));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return arc<uniform_buffer>{};
    }
}
} // end namespace kb
