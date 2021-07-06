#pragma once
#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Window.h"
#include "Kablunk/Core/LayerStack.h"
#include "Kablunk/Core/Timestep.h"

#include "Kablunk/Events/ApplicationEvent.h"

#include "Kablunk/Imgui/ImGuiLayer.h"

int main(int argc, char** argv);

namespace Kablunk {

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		static Application& Get() { return *s_Instance; }
		Window& GetWindow() { return *m_window; }

		void Close();

		ImGuiLayer* GetImGuiLayer() { return m_imgui_layer; }

		void SetWindowTitle(const std::string& title) { m_window->SetWindowTitle(title); }
		glm::vec2 GetWindowDimensions() const { return m_window->GetDimensions(); }
	private:
		bool OnWindowClosed(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		Scope<Window> m_window;
		ImGuiLayer* m_imgui_layer;
		bool m_running = true;
		bool m_minimized = false;
		LayerStack m_layer_stack;
		
		float m_last_frame_time = 0.0f;
		friend int ::main(int argc, char** argv);
		
	private:
		static Application* s_Instance;
	};

	Application* CreateApplication();
}

