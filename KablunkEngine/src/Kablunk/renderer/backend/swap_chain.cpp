#include "kablunkpch.h"

#include "Kablunk/renderer/backend/swap_chain.h"
#include "Kablunk/renderer/Renderer.h"

#include "Kablunk/renderer/backend/vulkan/vulkan_swap_chain.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

auto swap_chain::create() noexcept -> std::unique_ptr<swap_chain>
{
    const auto backend_type = Renderer::get_render_backend_type();
    switch (backend_type)
    {
    case render_backend_type_t::vulkan:
    {
        return std::make_unique<vk::vulkan_swap_chain>();
    }
    default:
    {
        KB_CORE_ASSERT(
            false,
            "[swap_chain]: Unhandled render backend type {}", static_cast<u32>(backend_type)
        );
        return nullptr;
    }
    }
}

} // end namespace kb::render::backend
