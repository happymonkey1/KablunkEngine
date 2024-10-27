#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_STORAGE_BUFFER_SET_H
#define KABLUNK_RENDERER_BACKEND_STORAGE_BUFFER_SET_H

#include "Kablunk/Core/Core.h"

#include "Kablunk/renderer/backend/storage_buffer.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

class storage_buffer_set : public RefCounted
{
public:
	virtual ~storage_buffer_set() = default;

	virtual void create(u32 p_size, u32 p_binding) = 0;

	virtual arc<storage_buffer> get(u32 p_binding, u32 p_set, u32 p_frame) = 0;
	virtual void set(arc<storage_buffer> p_storage_buffer, u32 p_set = 0, u32 p_frame = 0) = 0;
	virtual void resize(u32 p_binding, u32 p_set, uint32_t p_new_size) = 0;

	static arc<storage_buffer_set> create(u32 p_frames);
};

} // end namespace kb::render::backend

#endif
