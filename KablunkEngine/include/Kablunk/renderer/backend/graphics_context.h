#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_GRAPHICS_CONTEXT_H
#define KABLUNK_RENDERER_BACKEND_GRAPHICS_CONTEXT_H

#include <Kablunk/Core/RefCounting.h>

#include "Kablunk/renderer/backend/swap_chain.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

class graphics_context : public RefCounted
{
public:
	virtual ~graphics_context() = default;

	virtual void init() = 0;
	virtual void swap_buffers() = 0;
	virtual void shutdown() = 0;

    virtual arc<swap_chain>& get_swap_chain() noexcept = 0;

	static arc<graphics_context> create(void* window);
};

} // end namespace kb::render::backend

#endif
