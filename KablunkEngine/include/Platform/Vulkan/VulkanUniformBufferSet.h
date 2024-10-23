#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_UNIFORM_BUFFER_SET_H
#define KABLUNK_PLATFORM_VULKAN_UNIFORM_BUFFER_SET_H

#include "Kablunk/Renderer/UniformBufferSet.h"

#include <map>

namespace kb
{ // start namespace kb
class VulkanUniformBufferSet final : public UniformBufferSet
{
public:
	VulkanUniformBufferSet(u32 p_size, u32 p_frames_in_flight);
	~VulkanUniformBufferSet() override = default;

	arc<uniform_buffer> get() override;
    arc<uniform_buffer> rt_get() override;
    arc<uniform_buffer> get(u32 p_frame) override;

	void set(arc<uniform_buffer> p_uniform_buffer, uint32_t p_frame = 0) override;
private:
	uint32_t m_frames_in_flight;
	std::map<uint32_t, arc<uniform_buffer>> m_uniform_buffers;
};
} // end namespace kb

#endif
