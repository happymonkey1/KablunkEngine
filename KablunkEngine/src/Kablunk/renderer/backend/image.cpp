#include "kablunkpch.h"

#include "Kablunk/Renderer/backend/image.h"

#include "kablunk/renderer/backend/vulkan/VulkanImage.h"
#include "Kablunk/Renderer/RendererAPI.h"

namespace kb::render::backend
{ // start namespace kb::render::backend
arc<image_2d> image_2d::create(const image_specification_t& specification, owning_buffer buffer)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    if constexpr (backend == backend::render_backend_type_t::vulkan)
    {
        // TODO: singleton bad
        const auto& context = Singleton<Renderer>::get().get_graphics_context();
        auto device = context.as<vk::vulkan_context>()->get_device();
        return static_cast<arc<image_2d>>(arc<vk::vulkan_image_2d>::Create(
            specification,
            weak_arc{ device }
        ));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return arc<image_2d>{};
    }
}

arc<image_2d> image_2d::create(const image_specification_t& specification, const void* data /*= nullptr*/)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    if constexpr (backend == render_backend_type_t::vulkan)
    {
        // TODO: singleton bad
        const auto& context = Singleton<Renderer>::get().get_graphics_context();
        auto device = context.as<vk::vulkan_context>()->get_device();
        return static_cast<arc<image_2d>>(arc<vk::vulkan_image_2d>::Create(specification, weak_arc{ device }));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return arc<image_2d>{};
    }
}

auto image_view::create(const image_view_specification& p_specification) noexcept -> arc<image_view>
{
    constexpr auto backend = Renderer::get_render_backend_type();
    if constexpr (backend == render_backend_type_t::vulkan)
    {
        // TODO: singleton bad
        const auto& context = Singleton<Renderer>::get().get_graphics_context();
        auto device = context.as<vk::vulkan_context>()->get_device();
        return static_cast<arc<image_view>>(arc<vk::vulkan_image_view>::Create(p_specification, weak_arc{ device }));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return arc<image_view>{};
    }
}

} // end namespace kb::backend
