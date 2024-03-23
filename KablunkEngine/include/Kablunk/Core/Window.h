#pragma once

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/Core/Core.h"
#include "Kablunk/Events/Event.h"

#include "glm/glm.hpp"
#include "Kablunk/Core/handle.h"
#include "Kablunk/Renderer/Texture.h"

#include <glm/glm.hpp>

namespace kb
{ // start namespace kb

struct WindowProps {
	std::string Title;
	uint32_t Width;
	uint32_t Height;
	bool Fullscreen;

	WindowProps(
        const std::string& title = "Kablunk Engine",
        uint32_t width = 1600,
        uint32_t height = 900,
        bool fullscreen = false
    )
		: Title{ title }, Width{ width }, Height{ height }, Fullscreen{ fullscreen }
    {}
};

enum class window_mode_t : u8
{
	windowed = 0,
	fullscreen,
	borderless_fullscreen
};

class Window
{
public:
	using EventCallbackFn = std::function<void(Event&)>;

    virtual ~Window() = default;

	virtual void PollEvents() = 0;
	virtual void OnUpdate() = 0;

	virtual unsigned int GetWidth() const = 0;
	virtual unsigned int GetHeight() const = 0;
	virtual glm::vec2 GetDimensions() const = 0;
    virtual const glm::vec2& get_current_dpi() const noexcept = 0;

	virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
	virtual void SetVsync(bool enabled) = 0;
	virtual bool IsVsync() const = 0;
	virtual bool is_fullscreen() const = 0;

	virtual void SetWindowTitle(const std::string& title) = 0;

	virtual void* GetNativeWindow() const = 0;

	// change the "window" mode (i.e. windowed, fullscreen, borderless fullscreen)
	virtual void set_window_mode(window_mode_t mode) = 0;

	virtual void swap_buffers() = 0;

    virtual cursor_handle create_cursor(ref<Texture2D>& p_texture, const glm::ivec2& p_hot_spot) noexcept = 0;
    virtual void set_cursor(cursor_handle p_cursor_handle) noexcept = 0;
    // set cursor back to OS specific default cursor
    virtual void set_default_cursor() noexcept = 0;

	static box<Window> Create(const WindowProps& props = WindowProps());
};

} // end namespace kb

