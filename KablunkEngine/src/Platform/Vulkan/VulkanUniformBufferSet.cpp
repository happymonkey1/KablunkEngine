#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanUniformBufferSet.h"
#include "Kablunk/Renderer/RenderCommand.h"
#include "Kablunk/Core/Core.h"

namespace kb
{

VulkanUniformBufferSet::VulkanUniformBufferSet(u32 p_size, u32 p_frames_in_flight)
	: m_frames_in_flight{ p_frames_in_flight }
{
    if (p_frames_in_flight == 0)
    {
        m_frames_in_flight = render::get_frames_in_flight();
    }

    for (u32 frame = 0; frame < m_frames_in_flight; ++frame)
    {
        m_uniform_buffers.emplace(frame, uniform_buffer::create(p_size));
    }
}

arc<uniform_buffer> VulkanUniformBufferSet::get()
{
    const auto frame = render::get_current_frame_index();
    KB_CORE_ASSERT(
        m_uniform_buffers.contains(frame),
        "[VulkanUniformBufferSet]: Does not contain frame '{}'!",
        frame
    );
    return m_uniform_buffers.at(frame);
}

arc<uniform_buffer> VulkanUniformBufferSet::rt_get()
{
    const auto rt_frame = render::rt_get_current_frame_index();
    KB_CORE_ASSERT(
        m_uniform_buffers.contains(rt_frame),
        "[VulkanUniformBufferSet]: Does not contain frame '{}'!",
        rt_frame
    );
    return m_uniform_buffers.at(rt_frame);
}

arc<uniform_buffer> VulkanUniformBufferSet::get(u32 p_frame)
{
    KB_CORE_ASSERT(
        m_uniform_buffers.contains(p_frame),
        "[VulkanUniformBufferSet]: Does not contain frame '{}'!",
        p_frame
    );
    return m_uniform_buffers.at(p_frame);
}

void VulkanUniformBufferSet::set(arc<uniform_buffer> p_uniform_buffer, uint32_t p_frame)
{
    m_uniform_buffers.emplace(p_frame, p_uniform_buffer);
}

}
