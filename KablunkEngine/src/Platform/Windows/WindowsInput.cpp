#include "kablunkpch.h"

#include "Kablunk/Core/Input.h"

#include "Kablunk/Core/Application.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>

namespace Kablunk {

	bool Input::IsKeyPressed(int keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonPressed(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		bool imgui_enabled = Application::Get().GetSpecification().Enable_imgui;
		if (!imgui_enabled)
		{
			auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			return { (float)xpos, (float)ypos };
		}

		ImGuiContext* context = ImGui::GetCurrentContext();
		double xpos = -1, ypos = -1;
		ImGuiWindow* imgui_window = context->HoveredWindow;
		if (!imgui_window)
			return std::make_pair(0.0f, 0.0f);
		ImGuiViewport* viewport = imgui_window->Viewport;

		GLFWwindow* window = static_cast<GLFWwindow*>(viewport->PlatformHandle);
		if (!window)
		{
			KB_CORE_ERROR("could not find window!");
			return std::make_pair(0, 0);
		}

		double mx, my;
		glfwGetCursorPos(window, &mx, &my);
		ImVec2 window_pos = imgui_window->DC.CursorStartPos;
		ImVec2 viewport_pos = viewport->Pos;
		ImVec2 relative_window_pos = { window_pos.x - viewport_pos.x, window_pos.y - viewport_pos.y };
		ImVec2 window_size = imgui_window->Size;
		mx = mx - relative_window_pos.x;
		my = my - relative_window_pos.y;


		return std::make_pair(static_cast<float>(mx), static_cast<float>(my));
	}

	float Input::GetMouseX()
	{
		auto [x, y] = GetMousePosition();
		return x;
	}
	float Input::GetMouseY()
	{
		auto [x, y] = GetMousePosition();
		return y;
	}
}
