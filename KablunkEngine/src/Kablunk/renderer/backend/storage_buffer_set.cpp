#include "kablunkpch.h"

#include "Kablunk/Renderer/backend/storage_buffer_set.h"

#include "Kablunk/Renderer/Renderer.h"

#include "kablunk/renderer/backend/vulkan/vulkan_storage_buffer_set.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

arc<storage_buffer_set> storage_buffer_set::create(uint32_t p_frames)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    switch (backend)
    {
    case render_backend_type_t::vulkan:
        return static_cast<arc<storage_buffer_set>>(arc<vk::vulkan_storage_buffer_set>::Create(p_frames));
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return arc<storage_buffer_set>{};
    }
    }
}

} // end namespace kb::render
