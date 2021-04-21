#pragma once
#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Window.h"
#include "Kablunk/Core/LayerStack.h"
#include "Kablunk/Core/Timestep.h"

#include "Kablunk/Events/ApplicationEvent.h"

#include "Kablunk/Imgui/ImGuiLayer.h"

#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Buffer.h"
#include "Kablunk/Renderer/VertexArray.h"

namespace Kablunk {

	class KABLUNK_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
	private:
		bool OnWindowClosed(WindowCloseEvent& e);

	private:
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;
		
		float m_LastFrameTime = 0.0f;
		
		
	private:
		static Application* s_Instance;
	};

	Application* CreateApplication();
}

