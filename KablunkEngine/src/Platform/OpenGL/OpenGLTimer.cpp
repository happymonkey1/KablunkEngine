#include "kablunkpch.h"
#include "Platform/OpenGL/OpenGLTimer.h"

#include <glfw/glfw3.h>

namespace Kablunk
{

	OpenGLTimer::OpenGLTimer()
	{

	}

	float OpenGLTimer::GetTime() const
	{
		return static_cast<float>(glfwGetTime());
	}

}