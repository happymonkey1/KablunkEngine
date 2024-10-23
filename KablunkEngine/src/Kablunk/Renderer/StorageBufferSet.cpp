#include "kablunkpch.h"

#include "Kablunk/Renderer/StorageBufferSet.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanStorageBufferSet.h"

namespace kb
{ // start namespace kb
arc<StorageBufferSet> StorageBufferSet::Create(uint32_t frames)
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    switch (backend)
    {
    case render::render_backend_type_t::vulkan:
        return static_cast<arc<StorageBufferSet>>(arc<VulkanStorageBufferSet>::Create(frames));
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return arc<StorageBufferSet>{};
    }
    }
}
} // end namespace kb
