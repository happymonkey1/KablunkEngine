#include "kablunkpch.h"
#include "Kablunk/Core/Application.h"


#include "Kablunk/Events/Event.h"

#include "Kablunk/Renderer/Renderer.h"

#include "Kablunk/Core/Input.h"
#include "Platform/PlatformAPI.h"

namespace Kablunk 
{

	Application* Application::s_Instance = nullptr;

	

	Application::Application() 
	{
		KB_PROFILE_FUNCTION();

		KB_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		{
			m_window = Window::Create();
			m_window->SetEventCallback(KABLUNK_BIND_EVENT_FN(Application::OnEvent));
			m_window->SetVsync(false);
		}
		

		m_imgui_layer = new ImGuiLayer();
		PushOverlay(m_imgui_layer);

		Renderer::Init();
	}

	Application::~Application() {

	}

	void Application::PushLayer(Layer* layer) {
		KB_PROFILE_FUNCTION();

		m_layer_stack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay) {
		KB_PROFILE_FUNCTION();

		m_layer_stack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::Close()
	{
		m_running = false;
	}

	void Application::OnEvent(Event& e) {
		KB_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(KABLUNK_BIND_EVENT_FN(Application::OnWindowClosed));
		dispatcher.Dispatch<WindowResizeEvent>(KABLUNK_BIND_EVENT_FN(Application::OnWindowResize));


		for (auto it = m_layer_stack.rbegin(); it != m_layer_stack.rend(); ++it) {
			if (e.GetStatus())
				break;

			(*it)->OnEvent(e);
		}
	}

	bool Application::OnWindowClosed(WindowCloseEvent& e) {
		m_running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e) {
		KB_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_minimized = true;
			return false;
		}

		m_minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}
	

	void Application::Run() {
		KB_PROFILE_FUNCTION();

		while (m_running) 
		{
			KB_PROFILE_SCOPE("RunLoop - Application::Run")

			float time = PlatformAPI::GetTime(); // Platform::GetTime
			Timestep timestep = time - m_last_frame_time;
			m_last_frame_time = time;

			if (!m_minimized)
			{
				{
					KB_PROFILE_SCOPE("Layer OnUpdate - Application::Run")
					for (Layer* layer : m_layer_stack)
						layer->OnUpdate(timestep);
				}
			

				m_imgui_layer->Begin();
				{
					KB_PROFILE_SCOPE("Layer OnImGuiRender - Application::Run")
						for (Layer* layer : m_layer_stack)
							layer->OnImGuiRender(timestep);
				}
				m_imgui_layer->End();
			}

			m_window->OnUpdate();
		}
	}
}
