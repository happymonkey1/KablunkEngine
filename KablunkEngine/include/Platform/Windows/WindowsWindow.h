#pragma once

#include "Kablunk/Core/Window.h"


#include "Kablunk/Renderer/GraphicsContext.h"
#include "GLFW/glfw3.h"

struct GLFWwindow;

namespace kb
{ // start namespace kb
class WindowsWindow : public Window
{
public:
    static constexpr size_t k_max_cursors = 16ull;
public:
	WindowsWindow(const WindowProps& props);
	virtual ~WindowsWindow();

	virtual void PollEvents() override;
	virtual void OnUpdate() override;

	virtual unsigned int GetWidth() const override { return m_data.Width; }
	virtual unsigned int GetHeight()	const override { return m_data.Height; }
	virtual glm::vec2 GetDimensions() const override { return { m_data.Width, m_data.Height }; }
    virtual const glm::vec2& get_current_dpi() const noexcept override;

	virtual void SetEventCallback(const EventCallbackFn& callback) override { m_data.EventCallback = callback; }
	virtual void SetVsync(bool enabled) override;
	virtual bool IsVsync() const override;
	virtual bool is_fullscreen() const override { return m_data.Fullscreen; }

	virtual void SetWindowTitle(const std::string& title) override;

	virtual void* GetNativeWindow() const { return m_window; }


	// change the "window" mode (i.e. windowed, fullscreen, borderless fullscreen)
	virtual void set_window_mode(window_mode_t mode) override;

	virtual void swap_buffers() override;

    cursor_handle create_cursor(ref<Texture2D>& p_texture, const glm::ivec2& p_hot_spot) noexcept override;
    void set_cursor(cursor_handle p_cursor_handle) noexcept override;
    void set_default_cursor() noexcept override;

private:
	virtual void Init(const WindowProps& props);
	virtual void Shutdown();

    static auto compute_dpi(const glm::vec2& p_monitor_resolution, const glm::vec2& p_monitor_dimensions) noexcept -> glm::vec2;
private:
	GLFWwindow* m_window;
	ref<GraphicsContext> m_context;

	struct WindowData {
		std::string Title;
		unsigned int Width{ 0 }, Height{ 0 };
		bool VSync{ false };
		bool Fullscreen{ false };
        glm::vec2 m_current_dpi{ 0.f };

		EventCallbackFn EventCallback;
	} m_data;

    std::array<GLFWcursor*, k_max_cursors> m_cursors{};
    size_t m_cursor_count = 0ull;
};
} // end namespace kb

