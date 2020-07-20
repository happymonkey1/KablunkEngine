#include "Window.h"
#include <iostream>


namespace kablunkEngine { namespace graphics {
	void window_resize(GLFWwindow* window, int width, int height);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);


	Window::Window(const char* name, int width, int height) {
		m_Name = name;
		m_Width = width;
		m_Height = height;

		if (!init())
			glfwTerminate();

		for (int i = 0; i < MAX_KEYS; i++)
			m_Keys[i] = false;

		for (int i = 0; i < MAX_BUTTONS; i++)
			m_MouseButtons[i] = false;
	}

	Window::~Window() {
		glfwTerminate();
	}

	bool Window::init() {

		if (!glfwInit()) {
			std::cout << "Failed to initialize GLFW" << std::endl;
			return false;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


		m_Window = glfwCreateWindow(m_Width, m_Height, m_Name, NULL, NULL);
		if (!m_Window) {
			glfwTerminate();
			std::cout << "Window failed to create!" << std::endl;
			return false;
		}

		glfwMakeContextCurrent(m_Window);

		

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return false;
		}
		std::cout << "OpenGL" << glGetString(GL_VERSION) << std::endl;
		
		glfwSetWindowUserPointer(m_Window, this);
		glfwSetKeyCallback(m_Window, key_callback);
		glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
		glfwSetCursorPosCallback(m_Window, cursor_position_callback);
		glfwSetWindowSizeCallback(m_Window, window_resize);
		
		return true;
	}

	bool Window::isKeyPressed(unsigned int keycode) const {
		if (keycode >= MAX_KEYS)
			return false;
		else
			return m_Keys[keycode];	
	}

	bool Window::isMouseButtonPressed(unsigned int button) const {
		if (button >= MAX_BUTTONS)
			return false;
		else
			return m_MouseButtons[button];
	}

	void Window::getMousePosition(double& x, double& y) const {
		x = mx;
		y = my;
	}

	void Window::clear() const {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Window::update() {
		
		clear();

		glfwPollEvents();
		
		glfwSwapBuffers(m_Window);
	}

	bool Window::closed() const {
		return glfwWindowShouldClose(m_Window) == 1;
	}

	void window_resize(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	}

	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		Window* win = (Window*) glfwGetWindowUserPointer(window);
		
		win->m_Keys[key] = action != GLFW_RELEASE;
	}

	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
		Window* win = (Window*)glfwGetWindowUserPointer(window);

		win->m_MouseButtons[button] = action != GLFW_RELEASE;
	}

	void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
		Window* win = (Window*)glfwGetWindowUserPointer(window);

		win->mx = xpos;
		win->my = ypos;
	}
} };