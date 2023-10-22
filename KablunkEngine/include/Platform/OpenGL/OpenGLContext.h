#pragma once
#include "Kablunk/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace kb {
	class OpenGLContext : public GraphicsContext {
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		void Init() override;
		void SwapBuffers() override;
		void Shutdown() override;
	private:
		GLFWwindow* m_WindowHandle;
	};
}
