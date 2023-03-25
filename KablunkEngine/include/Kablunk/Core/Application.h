#pragma once
#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Window.h"
#include "Kablunk/Core/LayerStack.h"
#include "Kablunk/Core/Timestep.h"

#include "Kablunk/Events/ApplicationEvent.h"

#include "Kablunk/Imgui/ImGuiLayer.h"
#include "Kablunk/Core/ThreadPool.h"

#include "Kablunk/Core/RenderThread.h"

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
		// threading policy for the core engine
		threading_policy_t m_engine_threading_policy = threading_policy_t::multi_threaded;
	};

	class Application
	{
	public:
		struct performance_timings_t
		{
			f32 main_thread_work_time = 0.0f;
			f32 main_thread_wait_time = 0.0f;
			f32 render_thread_work_time = 0.0f;
			f32 render_thread_wait_time = 0.0f;
		};
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

		// event function which triggers on a key released event event
		// #NOTE(Sean) should probably be a "key down" event, but because KeyPressedEvent repeats, this is not useful when toggling fullscreen
		bool on_key_released(KeyReleasedEvent& e);

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

		render_thread& get_render_thread() { return m_render_thread; }
		
		// toggle between fullscreen and windowed mode
		void toggle_fullscreen();

		u32 get_current_frame_index() const { return m_current_frame_index; }

		performance_timings_t get_thread_performance_timings() const { return m_thread_performance_timings; }
	private:
		bool OnWindowClosed(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		ApplicationSpecification m_specification;
		Scope<Window> m_window;
		ImGuiLayer* m_imgui_layer;
		bool m_running = true;
		bool m_minimized = false;
		LayerStack m_layer_stack;
		
		// thread pool where tasks run
		Threading::ThreadPool m_thread_pool;
		// render thread
		render_thread m_render_thread;
		// records work and wait timings for threads
		performance_timings_t m_thread_performance_timings{};
		Timestep m_timestep;
		float m_last_frame_time = 0.0f;
		
		bool m_has_shutdown = false;

		u32 m_current_frame_index = 0;

		friend int ::main(int argc, char** argv);
	};

	// function defined as extern in entrypoint.h, client needs to implement
	Application* CreateApplication(int argc, char** argv);
}

