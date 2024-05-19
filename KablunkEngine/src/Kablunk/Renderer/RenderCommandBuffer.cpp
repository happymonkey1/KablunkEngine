#include "kablunkpch.h"

#include "Kablunk/Renderer/RendererAPI.h"
#include "Kablunk/Renderer/RenderCommandBuffer.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanRenderCommandBuffer.h"

namespace kb
{ // start namespace kb
ref<RenderCommandBuffer> RenderCommandBuffer::Create(uint32_t count /*= 0*/, const std::string& debug_name /*= ""*/)
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    switch (backend)
    {
    case render::render_backend_type_t::vulkan:
        return static_cast<ref<RenderCommandBuffer>>(ref<VulkanRenderCommandBuffer>::Create(count, debug_name));
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return ref<RenderCommandBuffer>{};
    }
    }
}

ref<RenderCommandBuffer> RenderCommandBuffer::CreateFromSwapChain(const std::string& debug_name /*= ""*/)
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    switch (backend)
    {
    case render::render_backend_type_t::vulkan:
        return static_cast<ref<RenderCommandBuffer>>(ref<VulkanRenderCommandBuffer>::Create(debug_name, true));
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return ref<RenderCommandBuffer>{};
    }
    }
}
} // end namespace kb
