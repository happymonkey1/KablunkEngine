#include "kablunkpch.h"
#include "OpenGLContext.h"


#include "glad/glad.h"
#include "GLFW/glfw3.h"

namespace Kablunk {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle{windowHandle}
	{
		if (m_WindowHandle == nullptr) KB_CORE_FATAL("Window handle is null");
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			KB_CORE_FATAL("GLAD FAILED TO INITIALIZE");

		KB_CORE_INFO("OpengGL Info:");
		KB_CORE_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
		KB_CORE_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
		KB_CORE_INFO("  Version: {0}", glGetString(GL_VERSION));
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}

}