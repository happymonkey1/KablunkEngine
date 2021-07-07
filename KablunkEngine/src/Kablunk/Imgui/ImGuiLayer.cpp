#include "kablunkpch.h"
#include "Kablunk/Imgui/ImGuiLayer.h"

#include "imgui.h"

#include "Kablunk/Core/Application.h"
#include "Kablunk/Events/KeyEvent.h"
#include "Kablunk/Events/MouseEvent.h"

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"

//TEMPORARY

#include <glad/glad.h>
#include <GLFW/glfw3.h>


namespace Kablunk {
	ImGuiLayer::ImGuiLayer() 
		: Layer{ "ImGuiLayer" }, m_time{ 0.0f } 
	{

	}

	ImGuiLayer::~ImGuiLayer()
	{

	}

	void ImGuiLayer::OnAttach()
	{
		KB_PROFILE_FUNCTION();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;


		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

		// #TODO build font library to load fonts instead of current way
		io.Fonts->AddFontFromFileTTF("assets/fonts/poppins/Poppins-Bold.ttf", 18.0f);
		io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/poppins/Poppins-Medium.ttf", 18.0f);
		//io.Fonts->Build();

		//ImGui::StyleColorsDark();
		SetDarkTheme();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			//style.Colors[ImGuiCol_Button] = { 0.381f, 0.381f, 0.381f, 1.0f };
		}

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		
		ImGui_ImplOpenGL3_Init("#version 450");
		ImGui_ImplGlfw_InitForOpenGL(window, true);
	}

	void ImGuiLayer::OnDetach()
	{
		KB_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::Begin()
	{
		KB_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End()
	{
		KB_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2(static_cast<float>(app.GetWindow().GetWidth()), static_cast<float>(app.GetWindow().GetHeight()));

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			
			// WINDOWS SPECIFIC CODE
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		if (!m_allow_event_passing)
		{
			ImGuiIO& io = ImGui::GetIO();

			auto handled = e.GetStatus();
			handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;

			e.SetStatus(handled);
		}
	}

	void ImGuiLayer::OnImGuiRender(Timestep ts)
	{
		/*static bool show = false;
		ImGui::ShowDemoWindow(&show);*/
		//ImGui::ShowStyleEditor();
	}

	void ImGuiLayer::SetDarkTheme()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_WindowBg]				= { 0.133f, 0.133f, 0.133f, 1.0f };	// #222222

		style.Colors[ImGuiCol_Header]				= { 0.200f, 0.205f, 0.210f, 1.0f };
		style.Colors[ImGuiCol_HeaderHovered]		= { 0.300f, 0.305f, 0.310f, 1.0f };
		style.Colors[ImGuiCol_HeaderActive]			= { 0.15f, 0.1505f, 0.151f, 1.0f };

		style.Colors[ImGuiCol_TitleBg]				= { 0.231f, 0.231f, 0.231f, 1.0f };	// #3B3B3B
		style.Colors[ImGuiCol_TitleBgActive]		= { 0.381f, 0.381f, 0.381f, 1.0f };	// #515151
		style.Colors[ImGuiCol_TitleBgCollapsed]		= { 0.15f, 0.1505f, 0.151f, 1.0f };

		style.Colors[ImGuiCol_Button]				= { 0.494f, 0.494f, 0.494f, 1.0f };
		style.Colors[ImGuiCol_ButtonHovered]		= { 0.300f, 0.305f, 0.310f, 1.0f };
		style.Colors[ImGuiCol_ButtonActive]			= { 0.15f, 0.1505f, 0.151f, 1.0f };

		style.Colors[ImGuiCol_FrameBg]				= { 0.494f, 0.494f, 0.494f, 1.0f };	// #7E7E7E
		style.Colors[ImGuiCol_FrameBgHovered]		= { 0.300f, 0.305f, 0.310f, 1.0f };
		style.Colors[ImGuiCol_FrameBgActive]		= { 0.15f, 0.1505f, 0.151f, 1.0f };

		style.Colors[ImGuiCol_Tab]					= { 0.15f, 0.1505f, 0.151f, 1.0f };
		style.Colors[ImGuiCol_TabHovered]			= { 0.38f, 0.3805f, 0.381f, 1.0f };
		style.Colors[ImGuiCol_TabActive]			= { 0.28f, 0.2805f, 0.281f, 1.0f };
		style.Colors[ImGuiCol_TabUnfocused]			= { 0.15f, 0.1505f, 0.151f, 1.0f };
		style.Colors[ImGuiCol_TabUnfocusedActive]	= { 0.200f, 0.205f, 0.210f, 1.0f };
	}
	

}
