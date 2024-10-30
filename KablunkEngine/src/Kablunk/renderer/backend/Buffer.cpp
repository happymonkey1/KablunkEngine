#include "kablunkpch.h"
#include "Kablunk/renderer/backend/buffer.h"

#include "Kablunk/Renderer/Renderer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_context.h"

#include "kablunk/renderer/backend/vulkan/vulkan_vertex_buffer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_index_buffer.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

arc<VertexBuffer> VertexBuffer::Create(uint32_t size)
{
	switch (Renderer::get_render_backend_type())
	{
	case render_backend_type_t::vulkan:
	{
        // TODO: singleton bad
        const auto context = Singleton<Renderer>::get().get_graphics_context();
        const auto device = context.as<vk::vulkan_context>()->get_device();
        return static_cast<arc<VertexBuffer>>(arc<vk::vulkan_vertex_buffer>::Create(
            weak_arc{ device },
            size
        ));
	}
    default:
    {
        KB_CORE_ASSERT(
            false,
            "[VertexBuffer::Create]: Unhandled render backend {}",
            static_cast<std::underlying_type_t<backend::render_backend_type_t>>(render::Renderer::get_render_backend_type())
        );
        return arc<VertexBuffer>{};
    }
	}
}

arc<VertexBuffer> VertexBuffer::Create(const void* data, uint32_t size)
{
    switch (Renderer::get_render_backend_type())
    {
    case render_backend_type_t::vulkan:
    {
        // TODO: singleton bad
        const auto context = Singleton<Renderer>::get().get_graphics_context();
        const auto device = context.as<vk::vulkan_context>()->get_device();
        return static_cast<arc<VertexBuffer>>(arc<vk::vulkan_vertex_buffer>::Create(
            weak_arc{ device },
            data,
            size
        ));
    }
    default:
    {
        KB_CORE_ASSERT(
            false,
            "[VertexBuffer::Create]: Unhandled render backend {}",
            static_cast<std::underlying_type_t<render_backend_type_t>>(render::Renderer::get_render_backend_type())
        );
        return arc<VertexBuffer>{};
    }
    }
}

arc<IndexBuffer> IndexBuffer::Create(uint32_t count)
{
    switch (Renderer::get_render_backend_type())
    {
    case render_backend_type_t::vulkan:
        return static_cast<arc<IndexBuffer>>(arc<backend::vk::vulkan_index_buffer>::Create(count));
    default:
    {
        KB_CORE_ASSERT(
            false,
            "[IndexBuffer::Create]: Unhandled render backend {}",
            static_cast<std::underlying_type_t<render_backend_type_t>>(render::Renderer::get_render_backend_type())
        );
        return arc<IndexBuffer>{};
    }
    }
}

arc<IndexBuffer> IndexBuffer::Create(const void* data, uint32_t count)
{
    switch (Renderer::get_render_backend_type())
    {
    case render_backend_type_t::vulkan:
        return static_cast<arc<IndexBuffer>>(arc<backend::vk::vulkan_index_buffer>::Create(data, count));
    default:
    {
        KB_CORE_ASSERT(
            false,
            "[IndexBuffer::Create]: Unhandled render backend {}",
            static_cast<std::underlying_type_t<render_backend_type_t>>(render::Renderer::get_render_backend_type())
        );
        return arc<IndexBuffer>{};
    }
    }
}

} // end namespace kb::render::backend
