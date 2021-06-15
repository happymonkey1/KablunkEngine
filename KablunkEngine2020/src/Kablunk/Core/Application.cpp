#include "kablunkpch.h"
#include "Kablunk/Core/Application.h"


#include "Kablunk/Events/Event.h"

#include "Kablunk/Renderer/Renderer.h"

#include "Platform/Windows/WindowsInput.h"
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
			m_Window = Window::Create();
			m_Window->SetEventCallback(KABLUNK_BIND_EVENT_FN(Application::OnEvent));
			m_Window->SetVsync(false);
		}
		

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		Renderer::Init();
	}

	Application::~Application() {

	}

	void Application::PushLayer(Layer* layer) {
		KB_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay) {
		KB_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::OnEvent(Event& e) {
		KB_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(KABLUNK_BIND_EVENT_FN(Application::OnWindowClosed));
		dispatcher.Dispatch<WindowResizeEvent>(KABLUNK_BIND_EVENT_FN(Application::OnWindowResize));


		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			if (e.GetStatus())
				break;
			(*--it)->OnEvent(e);
		}
	}

	bool Application::OnWindowClosed(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e) {
		KB_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}
	

	void Application::Run() {
		KB_PROFILE_FUNCTION();

		while (m_Running) 
		{
			KB_PROFILE_SCOPE("RunLoop - Application::Run")

			float time = PlatformAPI::GetTime(); // Platform::GetTime
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized)
			{
				{
					KB_PROFILE_SCOPE("Layer OnUpdate - Application::Run")
					for (Layer* layer : m_LayerStack)
						layer->OnUpdate(timestep);
				}
			}

			m_ImGuiLayer->Begin();
			{
				KB_PROFILE_SCOPE("Layer OnImGuiRender - Application::Run")
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender(timestep);
			}
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}
}