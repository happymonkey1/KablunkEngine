#include "kablunkpch.h"

#include "Kablunk/Renderer/backend/storage_buffer.h"

#include "Kablunk/renderer/renderer.h"
#include "Kablunk/renderer/backend/backend_type.h"
#include "Kablunk/renderer/backend/vulkan/vulkan_storage_buffer.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

arc<storage_buffer> storage_buffer::create(size_t size, uint32_t binding)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    if constexpr (backend == render_backend_type_t::vulkan)
    {
        return static_cast<arc<storage_buffer>>(arc<vk::vulkan_storage_buffer>::Create(size, binding));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return arc<storage_buffer>{};
    }
}

} // end namespace kb::render::backend
