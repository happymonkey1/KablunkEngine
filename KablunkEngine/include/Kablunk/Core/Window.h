#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Events/Event.h"

#include "glm/glm.hpp"

namespace Kablunk {

	struct WindowProps {
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const std::string& title = "Kablunk Engine", uint32_t width = 1600, uint32_t height = 900)
			: Title(title), Width(width), Height(height) { }
		
	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void PollEvents() = 0;
		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;
		virtual glm::vec2 GetDimensions() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVsync(bool enabled) = 0;
		virtual bool IsVsync() const = 0;

		virtual void SetWindowTitle(const std::string& title) = 0;

		virtual void* GetNativeWindow() const = 0;

		static Scope<Window> Create(const WindowProps& props = WindowProps());
	};
}

