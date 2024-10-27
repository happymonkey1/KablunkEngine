#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_SWAP_CHAIN_H
#define KABLUNK_RENDERER_BACKEND_SWAP_CHAIN_H

#include "Kablunk/Core/RefCounting.h"

// Forward declaration
struct GLFWwindow;

namespace kb::render::backend
{ // start namespace kb::render::backend

struct swap_chain_create_info_t
{
    u32 m_width = 1920;
    u32 m_height = 1080;
    bool m_vsync = false;
};

class swap_chain : public RefCounted
{
public:
    virtual ~swap_chain() noexcept = default;

    static auto create() noexcept -> arc<swap_chain>;

    // Initialize rendering surface (window) for the given rendering API
    virtual void init_surface(GLFWwindow* p_window) noexcept = 0;

    // Initialize swap chain for the given rendering API
    virtual void create(u32* p_width, u32* p_height, bool p_vsync) noexcept = 0;

    // Callback for when window resizes
    virtual void OnResize(u32 width, u32 height) = 0;

    // Per-frame initialization for the swap chain
    virtual void BeginFrame() noexcept = 0;

    // Presentation logic (submit rendered frame to display adaptor)
    virtual void Present() noexcept = 0;

    // Return the current frame buffer index for the swap chain
    virtual u32 GetCurrentBufferIndex() const noexcept = 0;

    // Retrieve the width of the presentation context
    virtual u32 GetWidth() const noexcept = 0;

    // Retrieve the height of the presentation context
    virtual u32 GetHeight() const noexcept = 0;

    // Destroy the swap chain and release any allocated memory
    virtual void Destroy() noexcept = 0;
};

} // end namespace kb::render::backend

#endif
