#pragma once
#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Window.h"
#include "Kablunk/Core/LayerStack.h"
#include "Kablunk/Core/Timestep.h"

#include "Kablunk/Events/ApplicationEvent.h"

#include "Kablunk/Imgui/ImGuiLayer.h"
#include "Kablunk/Core/ThreadPool.h"

#include "Kablunk/Core/RenderThread.h"

#include "Kablunk/Renderer/Renderer2D.h"

int main(int argc, char** argv);

namespace kb {

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
		Application(const ApplicationSpecification& spec);
		virtual ~Application();

		// initialize application based off application specification. Called by run function before main loop.
		void init();

		// shutdown application and release memory. Called by run function before exiting.
		void shutdown();

		// Set application specification for the application. Does not inherently modify application parameters.
		void set_application_specification(const ApplicationSpecification& specification)
		{
            m_specification = specification;
		    m_render_thread = render_thread{ specification.m_engine_threading_policy };
		}

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

        // get a mutable reference to the primary renderer2d
        ref<Renderer2D> get_renderer_2d() noexcept { return m_renderer_2d; }
        // get an immutable reference to the primary renderer2d
        const ref<Renderer2D>& get_renderer_2d() const noexcept { return m_renderer_2d; }
        // get a mutable reference to the secondary renderer2d
        ref<Renderer2D> get_screen_space_renderer_2d() noexcept { return m_screen_space_renderer_2d; }
        // get an immutable reference to the secondary renderer2d
        const ref<Renderer2D>& get_screen_space_renderer_2d() const noexcept { return m_screen_space_renderer_2d; }

		// toggle between fullscreen and windowed mode
		void toggle_fullscreen();
		// toggle debug statistics
		void draw_debug_statistics();

		u32 get_current_frame_index() const { return m_current_frame_index; }

		performance_timings_t get_thread_performance_timings() const { return m_thread_performance_timings; }
        performance_timings_t& get_thread_performance_timings_mut() { return m_thread_performance_timings; }

        auto get_current_dpi() const noexcept -> const glm::vec2& { return m_window->get_current_dpi(); }

        [[nodiscard]] auto create_cursor(
            ref<Texture2D>& p_texture,
            const glm::ivec2& p_hot_spot
        ) const noexcept -> cursor_handle
		{
            return m_window->create_cursor(p_texture, p_hot_spot);
		}

        auto set_cursor(cursor_handle p_cursor_handle) const noexcept -> void
		{
            m_window->set_cursor(p_cursor_handle);
		}

        auto set_default_cursor() const noexcept -> void { m_window->set_default_cursor(); }

	private:
		bool OnWindowClosed(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		ApplicationSpecification m_specification;
		box<Window> m_window;
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
		// flag for whether we should draw debug statistics to the screen
		bool m_show_debug_statistics = false;

        // primary 2d renderer which uses world space camera
        ref<Renderer2D> m_renderer_2d{};
        // secondary 2d renderer that operates in screen space (UI)
        ref<Renderer2D> m_screen_space_renderer_2d{};

		u32 m_current_frame_index = 0;

		friend int ::main(int argc, char** argv);
	};

	// function defined as extern in entrypoint.h, client needs to implement
	Application* CreateApplication(int argc, char** argv);
}

