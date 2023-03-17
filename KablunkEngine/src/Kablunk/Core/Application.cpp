#include "kablunkpch.h"
#include "Kablunk/Core/Application.h"

#include "Kablunk/Scripts/NativeScriptEngine.h"
#include "Kablunk/Events/Event.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/Renderer2D.h"
#include "Kablunk/Renderer/RenderCommand2D.h"

#include "Kablunk/Core/Input.h"
#include "Platform/PlatformAPI.h"

#include "Platform/Vulkan/VulkanContext.h"

//#include "Kablunk/Scripts/NativeScriptEngine.h"
#include "Kablunk/Plugin/PluginManager.h"
#include "Kablunk/Scripts/CSharpScriptEngine.h"

#include "Kablunk/Imgui/ImGuiGlobalContext.h"

#include "Kablunk/Asset/AssetManager.h"

#include "Kablunk/Audio/AudioCommand.h"



namespace Kablunk
{
	constexpr uint8_t NUM_JOB_THREADS = 4;


	Application::Application()
		: m_specification{}, m_thread_pool{ NUM_JOB_THREADS }, m_imgui_layer{ nullptr }, m_render_thread{ m_specification.m_engine_threading_policy }
	{
		
	}

	Application::~Application()
	{
		shutdown();
	}

	void Application::init()
	{
		KB_CORE_INFO("Application initialized");

		m_render_thread.run();

		KB_PROFILE_FUNCTION();
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
		render2d::init();
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
		CSharpScriptEngine::Init("Resources/Scripts/Kablunk-ScriptCore.dll");


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
		CSharpScriptEngine::Shutdown();

		// clear the framebuffer pool
		FramebufferPool::Get()->GetAll().clear();

		// deletes any pushed layers, including imgui layer
		m_layer_stack.Destroy();

		m_render_thread.terminate();

		ProjectManager::get().shutdown();

		render::shutdown();

		NativeScriptEngine::get().shutdown();
		PluginManager::get().shutdown();

		audio::shutdown_audio_engine();

		// delete window
		m_window.reset();

		m_has_shutdown = true;
	}

	void Application::PushLayer(Layer* layer)
	{
		KB_PROFILE_FUNCTION();

		m_layer_stack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		KB_PROFILE_FUNCTION();

		m_layer_stack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	bool Application::on_key_released(KeyReleasedEvent& e)
	{
		if (e.GetKeyCode() == Key::F11)
			toggle_fullscreen();

		return false;
	}

	void Application::Close()
	{
		m_running = false;
	}

	void Application::OnEvent(Event& e)
	{
		KB_PROFILE_FUNCTION();

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
		KB_PROFILE_FUNCTION();

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
		KB_PROFILE_FUNCTION();

		while (m_running)
		{
			KB_PROFILE_SCOPE("RunLoop - Application::Run");

			//KB_CORE_TRACE("Vsync: {0}", GetWindow().IsVsync());
			//if (NativeScriptEngine::Update())
			//	continue;

			{
				// #TODO profilers

				// synchronize threads
				m_render_thread.block_until_rendering_complete();
			}

			// poll events on main thread
			m_window->PollEvents();

			m_render_thread.next_frame();

			m_render_thread.kick();

			if (!m_minimized)
			{
				// #TODO(Sean) not renderer agnostic
				// start swapchain presentation on render thread
				render::submit([&]() { VulkanContext::Get()->GetSwapchain().BeginFrame(); });

				render::begin_frame();
				{
					KB_PROFILE_SCOPE("Layer OnUpdate - Application::Run")
						for (Layer* layer : m_layer_stack)
							layer->OnUpdate(m_timestep);
				}

				if (m_specification.Enable_imgui)
				{
					//KB_TIME_FUNCTION_BEGIN()
					Application* app = this;
					render::submit([app]() { app->RenderImGui(); });
					render::submit([=]() { m_imgui_layer->End(); });
					
					//KB_TIME_FUNCTION_END("imgui layer time")
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
			}

			float time = PlatformAPI::GetTime(); // Platform::GetTime
			m_timestep = time - m_last_frame_time;
			m_last_frame_time = time;
		}

		shutdown();
	}

	void Application::RenderImGui()
	{
		m_imgui_layer->Begin();
		{
			KB_PROFILE_SCOPE("Layer OnImGuiRender - Application::Run");
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

}
