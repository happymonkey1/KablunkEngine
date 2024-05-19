#include "kablunkpch.h"
#include "Kablunk/Renderer/Buffer.h"

#include "Kablunk/Renderer/Renderer.h"

#include "Platform/Vulkan/VulkanVertexBuffer.h"
#include "Platform/Vulkan/VulkanIndexBuffer.h"

namespace kb
{
	ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (render::Renderer::get_render_backend_type())
		{
		case render::render_backend_type_t::vulkan:
            return static_cast<ref<VertexBuffer>>(ref<VulkanVertexBuffer>::Create(size));
        default:
        {
            KB_CORE_ASSERT(
                false,
                "[VertexBuffer::Create]: Unhandled render backend {}",
                static_cast<std::underlying_type_t<render::render_backend_type_t>>(render::Renderer::get_render_backend_type())
            );
            return ref<VertexBuffer>{};
        }
		}
	}

	ref<VertexBuffer> VertexBuffer::Create(const void* data, uint32_t size)
	{
        switch (render::Renderer::get_render_backend_type())
        {
        case render::render_backend_type_t::vulkan:
            return static_cast<ref<VertexBuffer>>(ref<VulkanVertexBuffer>::Create(data, size));
        default:
        {
            KB_CORE_ASSERT(
                false,
                "[VertexBuffer::Create]: Unhandled render backend {}",
                static_cast<std::underlying_type_t<render::render_backend_type_t>>(render::Renderer::get_render_backend_type())
            );
            return ref<VertexBuffer>{};
        }
        }
	}

	ref<IndexBuffer> IndexBuffer::Create(uint32_t count)
	{
        switch (render::Renderer::get_render_backend_type())
        {
        case render::render_backend_type_t::vulkan:
            return static_cast<ref<IndexBuffer>>(ref<VulkanIndexBuffer>::Create(count));
        default:
        {
            KB_CORE_ASSERT(
                false,
                "[IndexBuffer::Create]: Unhandled render backend {}",
                static_cast<std::underlying_type_t<render::render_backend_type_t>>(render::Renderer::get_render_backend_type())
            );
            return ref<IndexBuffer>{};
        }
        }
	}

	ref<IndexBuffer> IndexBuffer::Create(const void* data, uint32_t count)
	{
        switch (render::Renderer::get_render_backend_type())
        {
        case render::render_backend_type_t::vulkan:
            return static_cast<ref<IndexBuffer>>(ref<VulkanIndexBuffer>::Create(data, count));
        default:
        {
            KB_CORE_ASSERT(
                false,
                "[IndexBuffer::Create]: Unhandled render backend {}",
                static_cast<std::underlying_type_t<render::render_backend_type_t>>(render::Renderer::get_render_backend_type())
            );
            return ref<IndexBuffer>{};
        }
        }
	}

}
