#include "kablunkpch.h"
#include "Kablunk/Renderer/Buffer.h"

#include "Kablunk/Renderer/Renderer.h"

#include "Platform/Vulkan/VulkanVertexBuffer.h"
#include "Platform/Vulkan/VulkanIndexBuffer.h"

namespace kb
{
	arc<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (render::Renderer::get_render_backend_type())
		{
		case render::render_backend_type_t::vulkan:
            return static_cast<arc<VertexBuffer>>(arc<VulkanVertexBuffer>::Create(size));
        default:
        {
            KB_CORE_ASSERT(
                false,
                "[VertexBuffer::Create]: Unhandled render backend {}",
                static_cast<std::underlying_type_t<render::render_backend_type_t>>(render::Renderer::get_render_backend_type())
            );
            return arc<VertexBuffer>{};
        }
		}
	}

	arc<VertexBuffer> VertexBuffer::Create(const void* data, uint32_t size)
	{
        switch (render::Renderer::get_render_backend_type())
        {
        case render::render_backend_type_t::vulkan:
            return static_cast<arc<VertexBuffer>>(arc<VulkanVertexBuffer>::Create(data, size));
        default:
        {
            KB_CORE_ASSERT(
                false,
                "[VertexBuffer::Create]: Unhandled render backend {}",
                static_cast<std::underlying_type_t<render::render_backend_type_t>>(render::Renderer::get_render_backend_type())
            );
            return arc<VertexBuffer>{};
        }
        }
	}

	arc<IndexBuffer> IndexBuffer::Create(uint32_t count)
	{
        switch (render::Renderer::get_render_backend_type())
        {
        case render::render_backend_type_t::vulkan:
            return static_cast<arc<IndexBuffer>>(arc<VulkanIndexBuffer>::Create(count));
        default:
        {
            KB_CORE_ASSERT(
                false,
                "[IndexBuffer::Create]: Unhandled render backend {}",
                static_cast<std::underlying_type_t<render::render_backend_type_t>>(render::Renderer::get_render_backend_type())
            );
            return arc<IndexBuffer>{};
        }
        }
	}

	arc<IndexBuffer> IndexBuffer::Create(const void* data, uint32_t count)
	{
        switch (render::Renderer::get_render_backend_type())
        {
        case render::render_backend_type_t::vulkan:
            return static_cast<arc<IndexBuffer>>(arc<VulkanIndexBuffer>::Create(data, count));
        default:
        {
            KB_CORE_ASSERT(
                false,
                "[IndexBuffer::Create]: Unhandled render backend {}",
                static_cast<std::underlying_type_t<render::render_backend_type_t>>(render::Renderer::get_render_backend_type())
            );
            return arc<IndexBuffer>{};
        }
        }
	}

}
