#include "kablunkpch.h"

#include "Kablunk/Renderer/backend/image.h"

#include "kablunk/renderer/backend/vulkan/vulkan_image.h"

namespace kb::render::backend
{ // start namespace kb::render::backend
arc<image_2d> image_2d::create(const image_specification_t& specification, owning_buffer buffer)
{
    switch (Singleton<Renderer>::get().get_render_backend_type())
    {
    case render_backend_type_t::vulkan:
    {
        // TODO: singleton bad
        const auto& context = Singleton<Renderer>::get().get_graphics_context();
        auto device = context.as<vk::vulkan_context>()->get_device();
        return static_cast<arc<image_2d>>(arc<vk::vulkan_image_2d>::Create(
            specification,
            weak_ptr{ device }
        ));
    }
    case render_backend_type_t::none:
        return arc<image_2d>{};
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return arc<image_2d>{};
    }
    }
}

arc<image_2d> image_2d::create(const image_specification_t& specification, const void* data /*= nullptr*/)
{
    switch (Singleton<Renderer>::get().get_render_backend_type())
    {
    case render_backend_type_t::vulkan:
    {
        // TODO: singleton bad
        const auto& context = Singleton<Renderer>::get().get_graphics_context();
        auto device = context.as<vk::vulkan_context>()->get_device();
        return static_cast<arc<image_2d>>(arc<vk::vulkan_image_2d>::Create(specification, weak_ptr{ device }));
    }
    case render_backend_type_t::none:
        return arc<image_2d>{};
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return arc<image_2d>{};
    }
    }
}

auto image_view::create(const image_view_specification& p_specification) noexcept -> arc<image_view>
{
    switch (Singleton<Renderer>::get().get_render_backend_type())
    {
    case render_backend_type_t::vulkan:
    {
        // TODO: singleton bad
        const auto& context = Singleton<Renderer>::get().get_graphics_context();
        auto device = context.as<vk::vulkan_context>()->get_device();
        return static_cast<arc<image_view>>(arc<vk::vulkan_image_view>::Create(p_specification, weak_ptr{ device }));
    }
    case render_backend_type_t::none:
        return arc<image_view>{};
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return arc<image_view>{};
    }
    }
}

} // end namespace kb::backend
