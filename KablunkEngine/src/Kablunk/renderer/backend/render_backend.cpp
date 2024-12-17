#include "kablunkpch.h"

#include "Kablunk/renderer/backend/render_backend.h"

#include "kablunk/renderer/backend/vulkan/vulkan_render_backend.h"
#include "kablunk/renderer/backend/vulkan/vulkan_context.h"


namespace kb::render::backend
{ // start namespace kb::render::backend

auto render_backend::create(
    render_backend_type_t p_render_backend_type,
    graphics_context* p_graphics_context
) noexcept -> render_backend*
{
    switch (p_render_backend_type)
    {
    case render_backend_type_t::vulkan:
    {
        const auto vulkan_context = weak_ptr{ p_graphics_context }.as<vk::vulkan_context>();
        return new vk::vulkan_render_backend{ vulkan_context };
    }
    case render_backend_type_t::none:
        return nullptr;
    }
}

} // end namespace kb::render::backend
