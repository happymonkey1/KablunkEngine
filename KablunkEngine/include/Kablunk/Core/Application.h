#pragma once
#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Window.h"
#include "Kablunk/Core/LayerStack.h"
#include "Kablunk/Core/Timestep.h"

#include "Kablunk/Events/ApplicationEvent.h"

#include "Kablunk/Imgui/ImGuiLayer.h"
#include "Kablunk/Core/ThreadPool.h"


int main(int argc, char** argv);

namespace Kablunk {

	struct ApplicationSpecification
	{
		std::string Name = "KablunkEngine";
		uint32_t Width = 1600, height = 900;
		bool Fullscreen = false;
		bool Vsync = false;
		// #NOTE use when projects are implemented
		std::string Working_directory = "";
		bool Resizable = true;
		bool Enable_imgui = true;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void Run();
		void OnEvent(Event& e);
		void OnStartup();
		void OnShutdown();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		static Application& Get() { return *s_Instance; }
		Window& GetWindow() { return *m_window; }
		const ApplicationSpecification& GetSpecification() const { return m_specification; }

		void Close();

		ImGuiLayer* GetImGuiLayer() { return m_imgui_layer; }

		void SetWindowTitle(const std::string& title) { m_window->SetWindowTitle(title); }
		glm::vec2 GetWindowDimensions() const { return m_window->GetDimensions(); }

		// #TODO create better api so this is not exposed to client applications
		Threading::ThreadPool& GetThreadPool() { return m_thread_pool; }
		const Threading::ThreadPool& GetThreadPool() const { return m_thread_pool; }
	private:
		bool OnWindowClosed(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		ApplicationSpecification m_specification;
		Scope<Window> m_window;
		ImGuiLayer* m_imgui_layer;
		bool m_running = true;
		bool m_minimized = false;
		LayerStack m_layer_stack;
		
		Threading::ThreadPool m_thread_pool;

		Timestep m_timestep;
		float m_last_frame_time = 0.0f;
		
		friend int ::main(int argc, char** argv);
		
	private:
		static Application* s_Instance;
	};

	// function defined as extern in entrypoint.h, client needs to implement
	Application* CreateApplication(int argc, char** argv);
}

