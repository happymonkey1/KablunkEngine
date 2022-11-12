#include "kablunkpch.h"

#include "Kablunk/Core/Input.h"

#include "Platform/Windows/WindowsWindow.h"
#include "Kablunk/Core/Application.h"

#include "Kablunk/Imgui/ImGuiGlobalContext.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "Kablunk/Renderer/RenderCommand.h"

namespace Kablunk::input
{

	bool is_key_pressed(int keycode)
	{
		bool imgui_enabled = Application::Get().GetSpecification().Enable_imgui;
		if (!imgui_enabled)
		{
			auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
			auto state = glfwGetKey(window, keycode);
			return state == GLFW_PRESS || state == GLFW_REPEAT;
		}

		ImGuiContext* context = ImGuiGlobalContext::get().get_context();
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

	bool is_mouse_button_pressed(int button)
	{
		bool imgui_enabled = Application::Get().GetSpecification().Enable_imgui;
		if (!imgui_enabled)
		{
			auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
			auto state = glfwGetMouseButton(window, button);
			return state == GLFW_PRESS;
		}

		ImGuiContext* context = ImGuiGlobalContext::get().get_context();
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

	bool is_mouse_in_viewport()
	{
		glm::vec2 mouse_pos = glm::vec2{ get_mouse_x_relative_to_viewport(), get_mouse_y_relative_to_viewport() };
		glm::vec2 window_pos = glm::vec2{ 0.0f };
		glm::vec2 window_size;

		if (Kablunk::Application::Get().GetSpecification().Enable_imgui)
		{
			window_pos = render::get_viewport_pos();
			window_size = render::get_viewport_size();
		}
		else
		{
			window_size = Application::Get().GetWindowDimensions();
		}


		bool x_true = mouse_pos.x >= window_pos.x && mouse_pos.x <= window_pos.x + window_size.x;
		bool y_true = mouse_pos.y >= window_pos.y && mouse_pos.y <= window_pos.y + window_size.y;

		return x_true && y_true;
	}

	std::pair<float, float> get_mouse_position_relative_to_viewport()
	{
		glm::vec2 mouse_pos = glm::vec2{ 0.0f };
		glm::vec2 window_pos = glm::vec2{ 0.0f };
		glm::vec2 window_size;

		if (Kablunk::Application::Get().GetSpecification().Enable_imgui)
		{
			// get imgui mouse pos (in screen coordinates)
			auto [x, y] = ImGui::GetMousePos();
			mouse_pos = glm::vec2{ x, y };

			window_pos = render::get_viewport_pos();
			window_size = render::get_viewport_size();
		}
		else
		{
			window_size = Application::Get().GetWindowDimensions();
			mouse_pos = glm::vec2{ input::get_mouse_x(), input::get_mouse_y() };
		}

		return std::make_pair(mouse_pos.x, mouse_pos.y);
	}

	float get_mouse_x_relative_to_viewport()
	{
		auto [x, y] = get_mouse_position_relative_to_viewport();
		return x;
	}

	float get_mouse_y_relative_to_viewport()
	{
		auto [x, y] = get_mouse_position_relative_to_viewport();
		return y;
	}

	std::pair<float, float> get_mouse_position()
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

	float get_mouse_x()
	{
		auto [x, y] = get_mouse_position();
		return x;
	}

	float get_mouse_y()
	{
		auto [x, y] = get_mouse_position();
		return y;
	}

	void set_cursor_mode(CursorMode mode)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL + static_cast<int>(mode));
	}

	CursorMode get_cursor_mode()
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		return static_cast<CursorMode>(glfwGetInputMode(window, GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
	}

	void set_mouse_motion_mode(MouseMotionMode mode)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		if (glfwRawMouseMotionSupported())
		{
			bool use_raw = mode == MouseMotionMode::Raw ? true : false;
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, use_raw);
		}
	}
	
	MouseMotionMode get_mouse_motion_mode()
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		int ret_code = glfwGetInputMode(window, GLFW_RAW_MOUSE_MOTION);

		return ret_code == GLFW_TRUE ? MouseMotionMode::Raw : MouseMotionMode::Normal;
	}
}
