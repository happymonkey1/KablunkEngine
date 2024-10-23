#include "kablunkpch.h"

#include "Kablunk/Renderer/Image.h"

#include "Platform/Vulkan/VulkanImage.h"
#include "Kablunk/Renderer/RendererAPI.h"

namespace kb
{ // start namespace kb
arc<Image2D> Image2D::Create(const ImageSpecification& specification, owning_buffer buffer)
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    if constexpr (backend == render::render_backend_type_t::vulkan)
    {
        return static_cast<arc<Image2D>>(arc<VulkanImage2D>::Create(specification));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return arc<Image2D>{};
    }
}

arc<Image2D> Image2D::Create(const ImageSpecification& specification, const void* data /*= nullptr*/)
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    if constexpr (backend == render::render_backend_type_t::vulkan)
    {
        return static_cast<arc<Image2D>>(arc<VulkanImage2D>::Create(specification));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return arc<Image2D>{};
    }
}

auto image_view::create(const image_view_specification& p_specification) noexcept -> arc<image_view>
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    if constexpr (backend == render::render_backend_type_t::vulkan)
    {
        return static_cast<arc<image_view>>(arc<vulkan_image_view>::Create(p_specification));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return arc<image_view>{};
    }
}

} // end namespace kb
