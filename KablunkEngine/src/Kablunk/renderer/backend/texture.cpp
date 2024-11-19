#include "kablunkpch.h"
#include "Kablunk/Renderer/backend/texture.h"

#include "Kablunk/Renderer/Renderer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_texture.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

arc<texture_2d> texture_2d::create(image_format_t format, uint32_t width, uint32_t height, const void* data)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    if constexpr (backend == render_backend_type_t::vulkan)
    {
        // TODO: singleton bad
        const auto context = Singleton<Renderer>::get().get_graphics_context();
        const auto device = context.as<vk::vulkan_context>()->get_device();
        return static_cast<arc<texture_2d>>(arc<vk::vulkan_texture_2d>::Create(
            device,
            format,
            width,
            height,
            data
        ));
    }
    else
    {
        KB_CORE_ASSERT(
            false,
            "[Texture2D::Create]: Unhandled render backend {}",
            static_cast<std::underlying_type_t<render_backend_type_t>>(backend)
        );
        return arc<texture_2d>{};
    }
}

arc<texture_2d> texture_2d::create(const std::string& path)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    if constexpr (backend == render_backend_type_t::vulkan)
    {
        // TODO: singleton bad
        const auto context = Singleton<Renderer>::get().get_graphics_context();
        const auto device = context.as<vk::vulkan_context>()->get_device();
        return static_cast<arc<texture_2d>>(arc<vk::vulkan_texture_2d>::Create(device, path));
    }
    else
    {
        KB_CORE_ASSERT(
            false,
            "[Texture2D::Create]: Unhandled render backend {}",
            static_cast<std::underlying_type_t<render_backend_type_t>>(backend)
        );
        return arc<texture_2d>{};
    }
}

} // end namespace kb::render::backend
