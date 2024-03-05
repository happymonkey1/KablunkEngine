#include "kablunkpch.h"
#include "Kablunk/Core/Application.h"

#include "Kablunk/Scripts/NativeScriptEngine.h"
#include "Kablunk/Events/Event.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include "Platform/Vulkan/VulkanContext.h"

#include "Kablunk/Core/Timers.h"
#include "Kablunk/Plugin/PluginManager.h"
#include "Kablunk/Scripts/CSharpScriptEngine.h"

#include "Kablunk/Imgui/ImGuiGlobalContext.h"

#include "Kablunk/Audio/AudioCommand.h"

#include <GLFW/glfw3.h>

namespace kb
{
constexpr uint8_t NUM_JOB_THREADS = 4;

Application::Application()
	: m_thread_pool{ NUM_JOB_THREADS }, m_imgui_layer{ nullptr }, m_render_thread{ m_specification.m_engine_threading_policy }
{
}

Application::Application(const ApplicationSpecification& spec)
	: m_specification{ spec }, m_thread_pool{ NUM_JOB_THREADS }, m_imgui_layer{ nullptr }, m_render_thread{ spec.m_engine_threading_policy }
{
}

Application::~Application()
{
	shutdown();
}

void Application::init()
{
    KB_PROFILE_SCOPE;
	KB_CORE_INFO("Application initialized");

	const char* thread_policy_cstr = m_render_thread.m_threading_policy == threading_policy_t::multi_threaded ? "multi-threaded" : "single-threaded";
	KB_CORE_INFO("engine threading policy {}", thread_policy_cstr);

	m_render_thread.run();

	{
		m_window = Window::Create({ m_specification.Name, m_specification.Width, m_specification.height, m_specification.Fullscreen });
		m_window->SetEventCallback([this](Event& e) { Application::OnEvent(e); });
		m_window->SetVsync(m_specification.Vsync);
	}


	audio::init_audio_engine();
	render::init();
	KB_CORE_INFO("Finished initializing renderer!");
	// start rendering render one frame
	m_render_thread.pump();

    m_renderer_2d = ref<Renderer2D>::Create();
    m_renderer_2d->init();

    m_screen_space_renderer_2d = ref<Renderer2D>::Create();
    m_screen_space_renderer_2d->init();

	m_render_thread.pump();

	//m_render_thread.pump();

	if (m_specification.Enable_imgui)
	{
		m_imgui_layer = ImGuiLayer::Create();
		PushOverlay(m_imgui_layer);

		// #TODO clean up so this is necessary
		// Get and set global ImGuiContext* for use in dll(s)
		ImGuiGlobalContext& g_imgui_context = ImGuiGlobalContext::get();
		g_imgui_context.init();
		g_imgui_context.set_context(ImGui::GetCurrentContext());
	}

	PluginManager::get().init();
	//CSharpScriptEngine::Init("Resources/Scripts/Kablunk-ScriptCore.dll");


	// #TODO should be based on projects later
#if KB_DEBUG
	constexpr const char* SANDBOX_PATH = "../bin/Debug-windows-x86_64/Sandbox/Sandbox.dll";
#else
	constexpr const char* SANDBOX_PATH = "../bin/Release-windows-x86_64/Sandbox/Sandbox.dll";
#endif
	//NativeScriptEngine::Open(SANDBOX_PATH);
}

void Application::shutdown()
{
	if (m_has_shutdown)
		return;

	m_thread_pool.Shutdown();
	//CSharpScriptEngine::Shutdown();

	// clear the framebuffer pool
	FramebufferPool::Get()->GetAll().clear();

	m_render_thread.terminate();

	// deletes any pushed layers, including imgui layer
	m_layer_stack.Destroy();

    m_renderer_2d.reset();
    m_screen_space_renderer_2d.reset();
	render::shutdown();

	ProjectManager::get().shutdown();

	NativeScriptEngine::get().shutdown();
	PluginManager::get().shutdown();

	audio::shutdown_audio_engine();

	// delete window
	m_window.reset();

	m_has_shutdown = true;
}

void Application::PushLayer(Layer* layer)
{
    KB_PROFILE_SCOPE;

	m_layer_stack.PushLayer(layer);
	layer->OnAttach();
}

void Application::PushOverlay(Layer* overlay)
{
    KB_PROFILE_SCOPE;

	m_layer_stack.PushOverlay(overlay);
	overlay->OnAttach();
}

bool Application::on_key_released(KeyReleasedEvent& e)
{
	if (e.GetKeyCode() == Key::F11)
		toggle_fullscreen();

	if (e.GetKeyCode() == Key::F3)
		m_show_debug_statistics = !m_show_debug_statistics;

	return false;
}

void Application::Close()
{
	m_running = false;
}

void Application::OnEvent(Event& e)
{
    KB_PROFILE_SCOPE;

	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return OnWindowClosed(e); });
	dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return OnWindowResize(e); });
	dispatcher.Dispatch<KeyReleasedEvent>([this](KeyReleasedEvent& e){ return on_key_released(e); });

	for (auto it = m_layer_stack.rbegin(); it != m_layer_stack.rend(); ++it) 
	{
		(*it)->OnEvent(e);
		if (e.GetStatus())
			break;
	}
}

bool Application::OnWindowClosed(WindowCloseEvent& e)
{
	m_running = false;
	return true;
}

bool Application::OnWindowResize(WindowResizeEvent& e)
{
    KB_PROFILE_SCOPE;

	const uint32_t width = e.GetWidth(), height = e.GetHeight();
	if (width == 0 || height == 0)
	{
		m_minimized = true;
		return false;
	}

	m_minimized = false;
	render::submit([&](){ render::on_window_resize(width, height); });
	return false;
}

void Application::Run()
{
    KB_PROFILE_SCOPE;

	while (m_running)
	{
		//KB_CORE_TRACE("Vsync: {0}", GetWindow().IsVsync());
		//if (NativeScriptEngine::Update())
		//	continue;

		{
			timer main_thread_wait_timer{};

			// synchronize threads
			m_render_thread.block_until_rendering_complete();
			m_thread_performance_timings.main_thread_wait_time = main_thread_wait_timer.get_elapsed_ms();
		}

		// poll events on main thread
		m_window->PollEvents();

        m_render_thread.next_frame();
        m_render_thread.kick();

		if (!m_minimized)
		{
			timer main_thread_cpu_timer{};

			// #TODO(Sean) not renderer agnostic
			// start swapchain presentation on render thread
			render::submit([&]() { VulkanContext::Get()->GetSwapchain().BeginFrame(); });

			render::begin_frame();
			{
				for (Layer* layer : m_layer_stack)
					layer->OnUpdate(m_timestep);
			}

			if (m_specification.Enable_imgui)
			{
				//KB_TIME_FUNCTION_BEGIN()
				auto app = this;
				render::submit([app]() { app->RenderImGui(); });
				render::submit([=]() { m_imgui_layer->End(); });

				//KB_TIME_FUNCTION_END("imgui layer time")
			}

			// draw debug statistics for engine
			{
				if (m_show_debug_statistics)
					draw_debug_statistics();
			}

			render::end_frame();

			render::submit([&](){ m_window->swap_buffers(); });

			// #TODO fix this so API agnostic
			/*if (RendererAPI::GetAPI() == RendererAPI::render_api_t::Vulkan)
			{
				VulkanContext::Get()->GetSwapchain().BeginFrame();
				render::wait_and_render();
			}*/

			//m_window->OnUpdate();

			m_current_frame_index = (m_current_frame_index + 1) % render::get_frames_in_flights();
			m_thread_performance_timings.main_thread_work_time = main_thread_cpu_timer.get_elapsed_ms();
		}

		float time = static_cast<float>(glfwGetTime()); // Platform::GetTime
		m_timestep = time - m_last_frame_time;
		m_last_frame_time = time;

        KB_FRAME_MARK;
	}

	shutdown();
}

void Application::RenderImGui()
{
    KB_PROFILE_SCOPE;
	m_imgui_layer->Begin();
	{
		for (Layer* layer : m_layer_stack)
			layer->OnImGuiRender(m_timestep);
	}
}

void Application::toggle_fullscreen()
{
	if (m_window->is_fullscreen())
		m_window->set_window_mode(window_mode_t::windowed);
	else
		m_window->set_window_mode(window_mode_t::borderless_fullscreen);
}

void Application::draw_debug_statistics()
{
#if 0
    // #TODO this should use "screen renderer" rather than world space renderer
	const auto& font_manager_ = m_renderer_2d->get_font_manager();
	ref<render::font_asset_t> font_asset = font_manager_.get_font_asset("Roboto-Medium.ttf");
	if (!font_asset)
	{
		KB_CORE_WARN("trying to draw debug statistics with an null font asset!");
		return;
	}

	Singleton<Renderer2D>::get().draw_text_string("test", glm::vec3{ 0.f, 0.f, 0.f }, glm::vec2{ 1.0f, 1.0f }, font_asset, glm::vec4{ 1.0f });
#endif
}

}
