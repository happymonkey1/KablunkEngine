#include "kablunkpch.h"

#include "Kablunk/Renderer/frame_buffer.h"
#include "Kablunk/Renderer/Renderer.h"

#include "Platform/Vulkan/vulkan_frame_buffer.h"

namespace kb::render
{ // start namespace kb::render
arc<frame_buffer> frame_buffer::create(const frame_buffer_specification& specs)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    arc<frame_buffer> frame_buffer{};
    switch (backend)
    {
    case render::render_backend_type_t::vulkan:
        frame_buffer = static_cast<arc<render::frame_buffer>>(arc<vulkan_frame_buffer>::Create(specs));
        break;
    default:
    {
        KB_CORE_ASSERT(
            false,
            "[Framebuffer::Create]: Unhandled render backend {}!",
            static_cast<std::underlying_type_t<render::render_backend_type_t>>(backend)
        );
        return arc<render::frame_buffer>{};
    }
    }

	//frame_buffer_pool::Get()->Add(frame_buffer);
	return frame_buffer;
}

#if 0
frame_buffer_pool* frame_buffer_pool::s_instance = new frame_buffer_pool{};
#else
frame_buffer_pool* frame_buffer_pool::s_instance = nullptr;
#endif

frame_buffer_pool::frame_buffer_pool(uint32_t max_framebuffers /*= 32*/)
{

}

frame_buffer_pool::~frame_buffer_pool() = default;

std::weak_ptr<frame_buffer> frame_buffer_pool::AllocateBuffer()
{
	return {};
}

void frame_buffer_pool::Add(const arc<frame_buffer>& framebuffer)
{
	m_pool.push_back(framebuffer);
}

} // end namespace kb::render
