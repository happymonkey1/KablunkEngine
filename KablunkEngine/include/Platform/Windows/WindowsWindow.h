#pragma once

#include "Kablunk/Core/Window.h"


#include "Kablunk/renderer/backend/swap_chain.h"
#include "kablunk/vendor/glfw/glfw.h"

struct GLFWwindow;

namespace kb
{ // start namespace kb
class WindowsWindow : public Window
{
public:
    static constexpr size_t k_max_cursors = 16ull;
public:
	WindowsWindow(const WindowProps& props, render::backend::swap_chain* p_swap_chain);
	~WindowsWindow() override;

	void PollEvents() override;
	void OnUpdate() override;

	unsigned int GetWidth() const override { return m_data.Width; }
	unsigned int GetHeight()	const override { return m_data.Height; }
	glm::vec2 GetDimensions() const override { return { m_data.Width, m_data.Height }; }
    const glm::vec2& get_current_dpi() const noexcept override;

	void SetEventCallback(const EventCallbackFn& callback) override { m_data.EventCallback = callback; }
	void SetVsync(bool enabled) override;
	bool IsVsync() const override;

	bool is_fullscreen() const override
	{
	    return m_data.Fullscreen;
	}

	void SetWindowTitle(const std::string& title) override;

	void* GetNativeWindow() const override { return m_window; }

	// change the "window" mode (i.e. windowed, fullscreen, borderless fullscreen)
	void set_window_mode(window_mode_t mode) override;

	void swap_buffers() override;

    cursor_handle create_cursor(arc<render::backend::texture_2d>& p_texture, const glm::ivec2& p_hot_spot) noexcept override;
    void set_cursor(cursor_handle p_cursor_handle) noexcept override;
    void set_default_cursor() noexcept override;

private:
	virtual void Init(const WindowProps& props, render::backend::swap_chain* p_swap_chain);
	virtual void Shutdown();

    static auto compute_dpi(const glm::vec2& p_monitor_resolution, const glm::vec2& p_monitor_dimensions) noexcept -> glm::vec2;
private:
	GLFWwindow* m_window;

    // Non-owning pointer to the swap chain
    render::backend::swap_chain* m_swap_chain;

	struct WindowData {
		std::string Title;
		unsigned int Width{ 0 }, Height{ 0 };
		bool VSync{ false };
		bool Fullscreen{ false };
        glm::vec2 m_current_dpi{ 0.f };
        window_mode_t m_window_mode = window_mode_t::windowed;

		EventCallbackFn EventCallback;
	} m_data;

    std::array<GLFWcursor*, k_max_cursors> m_cursors{};
    size_t m_cursor_count = 0ull;
};
} // end namespace kb

