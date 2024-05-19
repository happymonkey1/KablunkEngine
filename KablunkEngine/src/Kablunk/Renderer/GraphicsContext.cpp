#include "kablunkpch.h"
#include "Kablunk/Renderer/GraphicsContext.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace kb
{
	ref<GraphicsContext> GraphicsContext::Create(void* window)
	{
        constexpr auto backend = render::Renderer::get_render_backend_type();
        switch (backend)
        {
        case render::render_backend_type_t::vulkan:
            return static_cast<ref<GraphicsContext>>(ref<VulkanContext>::Create(static_cast<GLFWwindow*>(window)));
        default:
        {
            KB_CORE_ASSERT(
                false,
                "[GraphicsContext::Create]: Unhandled render backend {}",
                static_cast<std::underlying_type_t<render::render_backend_type_t>>(backend)
            );
            return ref<GraphicsContext>{};
        }
        }
	}
}
