#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_STORAGE_BUFFER_SET_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_STORAGE_BUFFER_SET_H

#include "Kablunk/Renderer/backend/storage_buffer_set.h"

#include <map>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

class vulkan_storage_buffer_set final : public storage_buffer_set
{
public:
	explicit vulkan_storage_buffer_set(uint32_t frames);
	~vulkan_storage_buffer_set() override = default;

	void create(uint32_t size, uint32_t binding) override;

	arc<storage_buffer> get(uint32_t binding, uint32_t set, uint32_t frame) override;
	void set(arc<storage_buffer> storage_buffer, uint32_t set = 0, uint32_t frame = 0) override;
	void resize(uint32_t binding, uint32_t set, uint32_t new_size) override;
private:
	uint32_t m_frames;
	std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, arc<storage_buffer>>>> m_storage_buffers;
};

} // end namespace kb::render::backend::vk

#endif
