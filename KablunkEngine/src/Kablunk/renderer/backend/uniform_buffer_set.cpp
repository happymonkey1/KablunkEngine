#include "kablunkpch.h"

#include "Kablunk/Renderer/backend/uniform_buffer_set.h"

#include "Kablunk/renderer/Renderer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_uniform_buffer_set.h"

namespace kb::render::backend
{ // start namespace kb::render::backend
arc<uniform_buffer_set> uniform_buffer_set::create(u32 p_size, u32 p_frames_in_flight)
{
    switch (Singleton<Renderer>::get().get_render_backend_type())
    {
    case render_backend_type_t::vulkan:
        return static_cast<arc<uniform_buffer_set>>(arc<vk::vulkan_uniform_buffer_set>::Create(p_size, p_frames_in_flight));
    case render_backend_type_t::none:
        return arc<uniform_buffer_set>{};
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return arc<uniform_buffer_set>{};
    }
    }
}
} // end namespace kb::render::backend
