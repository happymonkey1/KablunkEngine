#include "kablunkpch.h"

#include "Kablunk/Core/Input.h"

#include "Platform/Windows/WindowsWindow.h"
#include "Kablunk/Core/Application.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>

// #TODO temporary workaround for imgui context not being set in dll.
#define CHECK_IMGUI_WINDOW 0

namespace Kablunk {

	bool Input::IsKeyPressed(int keycode)
	{
#if CHECK_IMGUI_WINDOW
		bool imgui_enabled = Application::Get().GetSpecification().Enable_imgui;
#else
		bool imgui_enabled = false;
#endif
		if (!imgui_enabled)
		{
			auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
			auto state = glfwGetKey(window, keycode);
			return state == GLFW_PRESS || state == GLFW_REPEAT;
		}

		ImGuiContext* context = ImGui::GetCurrentContext();
		bool pressed = false;
		for (ImGuiViewport* viewport : context->Viewports)
		{
			if (!viewport->PlatformUserData)
				continue;

			GLFWwindow* window_handle = *(GLFWwindow**)viewport->PlatformUserData;
			if (!window_handle)
				continue;

			auto state = glfwGetKey(window_handle, static_cast<uint32_t>(keycode));
			if (state == GLFW_PRESS || state == GLFW_REPEAT)
			{
				pressed = true;
				break;
			}
		}

		return pressed;
	}

	bool Input::IsMouseButtonPressed(int button)
	{
#if CHECK_IMGUI_WINDOW
		bool imgui_enabled = Application::Get().GetSpecification().Enable_imgui;
#else
		bool imgui_enabled = false;
#endif
		if (!imgui_enabled)
		{
			auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
			auto state = glfwGetMouseButton(window, button);
			return state == GLFW_PRESS;
		}

		ImGuiContext* context = ImGui::GetCurrentContext();
		bool pressed = false;
		for (ImGuiViewport* viewport : context->Viewports)
		{
			if (!viewport->PlatformUserData)
				continue;

			GLFWwindow* window_handle = *(GLFWwindow**)viewport->PlatformUserData;
			if (!window_handle)
				continue;

			auto state = glfwGetMouseButton(window_handle, static_cast<uint32_t>(button));
			if (state == GLFW_PRESS)
			{
				pressed = true;
				break;
			}
		}

		return pressed;
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		//bool imgui_enabled = Application::Get().GetSpecification().Enable_imgui;
		//if (!imgui_enabled)
		//{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		return { (float)xpos, (float)ypos };
		//}

		/*
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


		return std::make_pair(static_cast<float>(mx), static_cast<float>(my));*/
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

	void Input::SetCursorMode(CursorMode mode)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL + static_cast<int>(mode));
	}

	CursorMode Input::GetCursorMode()
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		return static_cast<CursorMode>(glfwGetInputMode(window, GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
	}

	void Input::SetMouseMotionMode(MouseMotionMode mode)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		if (glfwRawMouseMotionSupported())
		{
			bool use_raw = mode == MouseMotionMode::Raw ? true : false;
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, use_raw);
		}
	}
	
	MouseMotionMode Input::GetMouseMotionMode()
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		int ret_code = glfwGetInputMode(window, GLFW_RAW_MOUSE_MOTION);

		return ret_code == GLFW_TRUE ? MouseMotionMode::Raw : MouseMotionMode::Normal;
	}
}
