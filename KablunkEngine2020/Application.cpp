#include "kablunkpch.h"
#include "Application.h"
#include "Event.h"
#include "ApplicationEvent.h"

#include <GLFW/glfw3.h>


namespace kablunk {

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)
	Application::Application() {
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
	}

	Application::~Application() {

	}

	void Application::OnEvent(Event& e) {
		KABLUNK_CORE_INFO(e);
	}

	void Application::Run() {
		while (m_Running) {
			glClearColor(1, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			m_Window->OnUpdate();
		}
	}
}