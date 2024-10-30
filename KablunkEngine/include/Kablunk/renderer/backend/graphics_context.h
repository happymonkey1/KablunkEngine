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
	~graphics_context() override = default;

    virtual void init() noexcept = 0;

	virtual void swap_buffers() = 0;

    // Return an non-owning, opaque pointer to the window handle
    virtual void* get_window_handle() const noexcept = 0;

    // Give the graphics context a non-owning pointer to the window handle
    virtual void set_window_handle(void* p_window_handle) noexcept = 0;

    virtual const swap_chain* get_swap_chain() const noexcept = 0;
    virtual swap_chain* get_swap_chain() noexcept = 0;

    // Destroy the graphics context, calling destruction logic for the underlying rendering API.
    // Releases any owned, allocated memory
    virtual void destroy() noexcept = 0;

	static arc<graphics_context> create(void* window);
};

} // end namespace kb::render::backend

#endif
