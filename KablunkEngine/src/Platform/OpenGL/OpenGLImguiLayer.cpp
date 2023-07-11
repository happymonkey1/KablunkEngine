#include "kablunkpch.h"

#include "Platform/OpenGL/OpenGLImguiLayer.h"

#include "Kablunk/Core/Application.h"
#include "Kablunk/Events/KeyEvent.h"
#include "Kablunk/Events/MouseEvent.h"

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

//TEMPORARY

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ImGuizmo.h>

namespace Kablunk
{
	OpenGLImguiLayer::OpenGLImguiLayer()
		: ImGuiLayer{}
	{

	}

	OpenGLImguiLayer::~OpenGLImguiLayer()
	{

	}

	void OpenGLImguiLayer::OnAttach()
	{
        KB_PROFILE_FUNC();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;


		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;


		// #TODO build font library to load fonts instead of current way
		io.Fonts->AddFontFromFileTTF("resources/fonts/poppins/Poppins-Bold.ttf", 18.0f);
		io.FontDefault = io.Fonts->AddFontFromFileTTF("resources/fonts/poppins/Poppins-Medium.ttf", 18.0f);
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

	void OpenGLImguiLayer::OnDetach()
	{
        KB_PROFILE_FUNC();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void OpenGLImguiLayer::OnUpdate(Timestep ts)
	{

	}

	void OpenGLImguiLayer::OnImGuiRender(Timestep ts)
	{
		/*static bool show = false;
		ImGui::ShowDemoWindow(&show);*/
		//ImGui::ShowStyleEditor();
	}

	void OpenGLImguiLayer::OnEvent(Event& event)
	{
		// #FIXME event passing
		/*if (!m_allow_event_passing)
		{
			ImGuiIO& io = ImGui::GetIO();

			auto handled = e.GetStatus();
			handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;

			e.SetStatus(handled);
		}*/
	}

	void OpenGLImguiLayer::Begin()
	{
        KB_PROFILE_FUNC();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void OpenGLImguiLayer::End()
	{
        KB_PROFILE_FUNC();

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
}
