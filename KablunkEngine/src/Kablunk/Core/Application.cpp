#include "kablunkpch.h"
#include "Kablunk/Core/Application.h"

#include "Kablunk/Scripts/NativeScriptEngine.h"
#include "Kablunk/Events/Event.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include "Kablunk/Core/Input.h"
#include "Platform/PlatformAPI.h"

#include "Platform/Vulkan/VulkanContext.h"

//#include "Kablunk/Scripts/NativeScriptEngine.h"
#include "Kablunk/Plugin/PluginManager.h"
#include "Kablunk/Scripts/CSharpScriptEngine.h"



namespace Kablunk
{

	Application* Application::s_Instance = nullptr;

	constexpr uint8_t NUM_JOB_THREADS = 4;


	Application::Application(const ApplicationSpecification& specification)
		: m_specification{ specification }, m_thread_pool{ NUM_JOB_THREADS }
	{
		KB_PROFILE_FUNCTION();

		KB_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		{
			m_window = Window::Create({ specification.Name, specification.Width, specification.height });
			m_window->SetEventCallback([this](Event& e) { Application::OnEvent(e); });
			m_window->SetVsync(specification.Vsync);
		}

		Renderer::Init();
		RenderCommand::WaitAndRender();

		if (specification.Enable_imgui)
		{
			m_imgui_layer = ImGuiLayer::Create();
			PushOverlay(m_imgui_layer);
		}

		Singleton<PluginManager>::init();
		CSharpScriptEngine::Init("Resources/Scripts/Kablunk-ScriptCore.dll");
		

		// #TODO should be based on projects later
#if KB_DEBUG
		constexpr const char* SANDBOX_PATH = "../bin/Debug-windows-x86_64/Sandbox/Sandbox.dll";
#else
		constexpr const char* SANDBOX_PATH = "../bin/Release-windows-x86_64/Sandbox/Sandbox.dll";
#endif
		//NativeScriptEngine::Open(SANDBOX_PATH);
	}

	Application::~Application()
	{
		m_thread_pool.Shutdown();
		CSharpScriptEngine::Shutdown();

		FramebufferPool::Get()->GetAll().clear();

		m_layer_stack.Destroy();

		RenderCommand::WaitAndRender();

		Renderer::Shutdown();

		Singleton<NativeScriptEngine>::shutdown();
		Singleton<PluginManager>::shutdown();
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
		Renderer::OnWindowResize(width, height);

		return false;
	}

	void Application::Run()
	{
		KB_PROFILE_FUNCTION();

		OnStartup();

		while (m_running)
		{
			KB_PROFILE_SCOPE("RunLoop - Application::Run");

			//KB_CORE_TRACE("Vsync: {0}", GetWindow().IsVsync());
			//if (NativeScriptEngine::Update())
			//	continue;

			m_window->PollEvents();

			if (!m_minimized)
			{
				RenderCommand::BeginFrame();
				{
					KB_PROFILE_SCOPE("Layer OnUpdate - Application::Run")
						for (Layer* layer : m_layer_stack)
							layer->OnUpdate(m_timestep);
				}

				if (m_specification.Enable_imgui)
				{
					//KB_TIME_FUNCTION_BEGIN()
					Application* app = this;
					RenderCommand::Submit([app]() { app->RenderImGui(); });
					RenderCommand::Submit([=]() { m_imgui_layer->End(); });
					
					//KB_TIME_FUNCTION_END("imgui layer time")
				}

				RenderCommand::EndFrame();

				// #TODO fix this so API agnostic
				if (RendererAPI::GetAPI() == RendererAPI::RenderAPI_t::Vulkan)
				{
					VulkanContext::Get()->GetSwapchain().BeginFrame();
					RenderCommand::WaitAndRender();
				}

				m_window->OnUpdate();
			}

			float time = PlatformAPI::GetTime(); // Platform::GetTime
			m_timestep = time - m_last_frame_time;
			m_last_frame_time = time;
		}

		OnShutdown();
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

	void Application::OnStartup()
	{

	}

	void Application::OnShutdown()
	{
		//NativeScriptEngine::Shutdown();
	}
}
