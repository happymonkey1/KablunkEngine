#pragma once

#include "Kablunk/Core/Window.h"


#include "Kablunk/Renderer/GraphicsContext.h"
#include "GLFW/glfw3.h"

struct GLFWwindow;

namespace Kablunk {
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		virtual void PollEvents() override;
		virtual void OnUpdate() override;

		virtual unsigned int GetWidth() const override { return m_data.Width; }

		virtual unsigned int GetHeight()	const override { return m_data.Height; }
		virtual glm::vec2 GetDimensions() const override { return { m_data.Width, m_data.Height }; }
		virtual void SetEventCallback(const EventCallbackFn& callback) override { m_data.EventCallback = callback; }
		virtual void SetVsync(bool enabled) override;
		virtual bool IsVsync() const override;

		virtual void SetWindowTitle(const std::string& title) override;

		virtual void* GetNativeWindow() const { return m_window; }
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();
	private:
		GLFWwindow* m_window;
		Scope<GraphicsContext> m_context;

		struct WindowData {
			std::string Title;
			unsigned int Width{ 0 }, Height{ 0 };
			bool VSync{ false };
			bool Fullscreen{ false };

			EventCallbackFn EventCallback;
		} m_data;
	};
}

