#include "kablunkpch.h"

#include "Kablunk/Renderer/Image.h"

#include "Platform/Vulkan/VulkanImage.h"
#include "Kablunk/Renderer/RendererAPI.h"

namespace kb
{ // start namespace kb
ref<Image2D> Image2D::Create(const ImageSpecification& specification, owning_buffer buffer)
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    if constexpr (backend == render::render_backend_type_t::vulkan)
    {
        return static_cast<ref<Image2D>>(ref<VulkanImage2D>::Create(specification));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return ref<Image2D>{};
    }
}

ref<Image2D> Image2D::Create(const ImageSpecification& specification, const void* data /*= nullptr*/)
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    if constexpr (backend == render::render_backend_type_t::vulkan)
    {
        return static_cast<ref<Image2D>>(ref<VulkanImage2D>::Create(specification));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return ref<Image2D>{};
    }
}

auto image_view::create(const image_view_specification& p_specification) noexcept -> ref<image_view>
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    if constexpr (backend == render::render_backend_type_t::vulkan)
    {
        return static_cast<ref<image_view>>(ref<vulkan_image_view>::Create(p_specification));
    }
    else
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!");
        return ref<image_view>{};
    }
}

} // end namespace kb
