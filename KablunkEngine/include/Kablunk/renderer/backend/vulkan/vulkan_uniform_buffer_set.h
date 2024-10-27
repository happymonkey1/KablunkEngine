#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_UNIFORM_BUFFER_SET_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_UNIFORM_BUFFER_SET_H

#include "Kablunk/renderer/backend/uniform_buffer_set.h"

#include <map>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

class vulkan_uniform_buffer_set final : public uniform_buffer_set
{
public:
	vulkan_uniform_buffer_set(u32 p_size, u32 p_frames_in_flight);
	~vulkan_uniform_buffer_set() override = default;

	arc<uniform_buffer> get() override;
    arc<uniform_buffer> rt_get() override;
    arc<uniform_buffer> get(u32 p_frame) override;

	void set(arc<uniform_buffer> p_uniform_buffer, uint32_t p_frame = 0) override;
private:
	uint32_t m_frames_in_flight;
	std::map<uint32_t, arc<uniform_buffer>> m_uniform_buffers;
};

} // end namespace kb::render::backend::vk

#endif
