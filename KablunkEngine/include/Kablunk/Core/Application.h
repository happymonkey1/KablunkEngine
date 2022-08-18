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
		Application();
		virtual ~Application();

		// initialize application based off application specification. Called by run function before main loop.
		void init();

		// shutdown application and release memory. Called by run function before exiting.
		void shutdown();

		// Set application specification for the application. Does not inherently modify application parameters.
		void set_application_specification(const ApplicationSpecification& specification) { m_specification = specification; }

		void Run();
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		static Application& Get() { return Singleton<Application>::get(); }
		Window& GetWindow() { return *m_window; }
		const ApplicationSpecification& GetSpecification() const { return m_specification; }

		void Close();

		ImGuiLayer* GetImGuiLayer() { return m_imgui_layer; }
		void RenderImGui();


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
	};

	// function defined as extern in entrypoint.h, client needs to implement
	Application* CreateApplication(int argc, char** argv);
}

