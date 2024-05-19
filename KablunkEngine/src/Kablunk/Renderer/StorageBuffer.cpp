#include "kablunkpch.h"

#include "Kablunk/Renderer/StorageBuffer.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Renderer/render_backend.h"

#include "Platform/Vulkan/VulkanStorageBuffer.h"

namespace kb
{ // start namespace kb
ref<StorageBuffer> StorageBuffer::Create(size_t size, uint32_t binding)
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    if constexpr (backend == render::render_backend_type_t::vulkan)
    {
        return static_cast<ref<StorageBuffer>>(ref<VulkanStorageBuffer>::Create(size, binding));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return ref<StorageBuffer>{};
    }
}
} // end namespace kb
