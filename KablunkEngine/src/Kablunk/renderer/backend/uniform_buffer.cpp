#include "kablunkpch.h"

#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Renderer/backend/uniform_buffer.h"

#include "Kablunk/renderer/Renderer.h"
#include "Kablunk/renderer/backend/backend_type.h"
#include "Kablunk/renderer/backend/vulkan/vulkan_uniform_buffer.h"

namespace kb::render::backend
{ // start namespace kb::render::backend
arc<uniform_buffer> uniform_buffer::create(u32 p_size)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    if constexpr (backend == render_backend_type_t::vulkan)
    {
        return static_cast<arc<uniform_buffer>>(arc<vk::vulkan_uniform_buffer>::Create(p_size));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return arc<uniform_buffer>{};
    }
}
} // end namespace kb::render::backend
