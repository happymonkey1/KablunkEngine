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

		virtual unsigned int GetWidth() const override { return m_Data.Width; }

		virtual unsigned int GetHeight()	const override { return m_Data.Height; }
		virtual glm::vec2 GetDimensions() const override { return { m_Data.Width, m_Data.Height }; }
		virtual void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		virtual void SetVsync(bool enabled) override;
		virtual bool IsVsync() const override;

		virtual void SetWindowTitle(const std::string& title) override;

		virtual void* GetNativeWindow() const { return m_Window; }
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();
	private:
		GLFWwindow* m_Window;
		Scope<GraphicsContext> m_Context;

		struct WindowData {
			std::string Title;
			unsigned int Width{ 0 }, Height{ 0 };
			bool VSync{ false };

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};
}

