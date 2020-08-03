#include "kablunkpch.h"
#include "Application.h"
#include "Event.h"


#include <GLFW/glfw3.h>


namespace kablunk {

#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)
	Application::Application() {
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
	}

	Application::~Application() {

	}

	void Application::PushLayer(Layer* layer) {
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* layer) {
		m_LayerStack.PushOverlay(layer);
	}

	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClosed));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			(*--it)->OnEvent(e);
			if (e.GetStatus())
				break;
		}
	}

	bool Application::OnWindowClosed(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}

	void Application::Run() {
		while (m_Running) {
			glClearColor(1, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			m_Window->OnUpdate();
		}
	}
}