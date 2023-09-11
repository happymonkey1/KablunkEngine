#include "kablunkpch.h"
#include "Platform/OpenGL/OpenGLContext.h"


#include "glad/glad.h"
#include "GLFW/glfw3.h"

namespace kb {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle{windowHandle}
	{
		if (m_WindowHandle == nullptr) KB_CORE_FATAL("Window handle is null");
	}

	void OpenGLContext::Init()
	{
        KB_PROFILE_FUNC();

		glfwMakeContextCurrent(m_WindowHandle);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			KB_CORE_FATAL("GLAD FAILED TO INITIALIZE");

		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		KB_CORE_ASSERT(status, "Failed to initialize Glad!");

		KB_CORE_INFO("OpengGL Info:");
		KB_CORE_INFO("  Vendor: {0}", (const char*)glGetString(GL_VENDOR));
		KB_CORE_INFO("  Renderer: {0}", (const char*)glGetString(GL_RENDERER));
		KB_CORE_INFO("  Version: {0}", (const char*)glGetString(GL_VERSION));

#ifdef KB_ENABLE_ASSERTS
		int versionMajor;
		int versionMinor;
		glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
		glGetIntegerv(GL_MINOR_VERSION, &versionMinor);

		KB_CORE_ASSERT(versionMajor > 4 || (versionMajor == 4 && versionMinor >= 5), "Kablunk requires at least OpenGL version 4.5!");
#endif
	}

	void OpenGLContext::SwapBuffers()
	{
        KB_PROFILE_FUNC();

		glfwSwapBuffers(m_WindowHandle);
	}

	void OpenGLContext::Shutdown()
	{

	}

}
