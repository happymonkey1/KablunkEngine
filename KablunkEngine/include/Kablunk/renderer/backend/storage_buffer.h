#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_STORAGE_BUFFER_H
#define KABLUNK_RENDERER_BACKEND_STORAGE_BUFFER_H

#include "Kablunk/Core/Core.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

class storage_buffer : public RefCounted
{
public:
	virtual ~storage_buffer() = default;

	virtual void set_data(const void* data, size_t size, uint32_t offset = 0) = 0;
	virtual void rt_set_data(const void* data, size_t size, uint32_t offset = 0) = 0;
	virtual void resize(size_t new_size) = 0;

	virtual uint32_t get_binding() = 0;

	static arc<storage_buffer> create(size_t size, uint32_t binding);
};

} // end namespace kb::render::backend

#endif
