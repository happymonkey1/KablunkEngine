#include "kablunkpch.h"
#include "Kablunk/Renderer/Texture.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanTexture.h"

namespace kb
{
	arc<Texture2D> Texture2D::Create(ImageFormat format, uint32_t width, uint32_t height, const void* data)
	{
        constexpr auto backend = render::Renderer::get_render_backend_type();
        if constexpr (backend == render::render_backend_type_t::vulkan)
        {
            return static_cast<arc<Texture2D>>(arc<VulkanTexture2D>::Create(format, width, height, data));
        }
        else
        {
            KB_CORE_ASSERT(
                false,
                "[Texture2D::Create]: Unhandled render backend {}",
                static_cast<std::underlying_type_t<render::render_backend_type_t>>(backend)
            );
            return arc<Texture2D>{};
        }
	}

	arc<Texture2D> Texture2D::Create(const std::string& path)
	{
        constexpr auto backend = render::Renderer::get_render_backend_type();
        if constexpr (backend == render::render_backend_type_t::vulkan)
        {
            return static_cast<arc<Texture2D>>(arc<VulkanTexture2D>::Create(path));
        }
        else
        {
            KB_CORE_ASSERT(
                false,
                "[Texture2D::Create]: Unhandled render backend {}",
                static_cast<std::underlying_type_t<render::render_backend_type_t>>(backend)
            );
            return arc<Texture2D>{};
        }
	}
}
