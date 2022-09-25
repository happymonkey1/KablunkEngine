#include "kablunkpch.h"
#include "Platform/Windows/WindowsWindow.h"


#include "Kablunk/Events/KeyEvent.h"
#include "Kablunk/Events/MouseEvent.h"
#include "Kablunk/Events/ApplicationEvent.h"

#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/OpenGL/OpenGLContext.h"
#include "Platform/Vulkan/VulkanContext.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Kablunk {

    static uint8_t s_glfw_window_count = 0;

    static void GLFWErrorCallback(int error, const char* desc) 
    {
        KB_CORE_ERROR("GLFW Error ({0} {1})", error, desc);
    }

    Scope<Window> Window::Create(const WindowProps& props) 
    {
        return CreateScope<WindowsWindow>(props);
    }

    WindowsWindow::WindowsWindow(const WindowProps& props)
    {
        KB_PROFILE_FUNCTION();

        Init(props);
    }

    WindowsWindow::~WindowsWindow()
    {
        KB_PROFILE_FUNCTION();

        Shutdown();
    }

    void WindowsWindow::Init(const WindowProps& props)
    {
        m_data.Title = props.Title;
        m_data.Width = props.Width;
        m_data.Height = props.Height;
		m_data.Fullscreen = props.Fullscreen;
        
        KB_CORE_INFO("Creating Window {0} ({1}x{2}), fullscreen={3}", props.Title, props.Width, props.Height, props.Fullscreen);
        

        if (s_glfw_window_count == 0) 
		{
			KB_PROFILE_SCOPE("glfwInit");

			int success = glfwInit();
            KB_CORE_ASSERT(success, "COULD NOT INITIALIZE GLFW");

			// Hint to glfw that this will be rendered with Vulkan
			if (RendererAPI::GetAPI() == RendererAPI::RenderAPI_t::Vulkan)
			{
				glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
				// #TODO resizing at runtime
				glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
			}

            glfwSetErrorCallback(GLFWErrorCallback);
        }

		{
			KB_PROFILE_SCOPE("glfwCreateWindow fullscreen");

			GLFWmonitor* primary_monitor = nullptr;
			if (m_data.Fullscreen)
			{
				primary_monitor = glfwGetPrimaryMonitor();
				const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);

				glfwWindowHint(GLFW_RED_BITS, mode->redBits);
				glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
				glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
				glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

				m_data.Width = mode->width;
				m_data.Height = mode->height;
			}

			m_window = glfwCreateWindow((int)m_data.Width, (int)m_data.Height, m_data.Title.c_str(), primary_monitor, nullptr);
			++s_glfw_window_count;
		}
		m_context = GraphicsContext::Create(m_window);
		m_context->Init();

		if (RendererAPI::GetAPI() == RendererAPI::RenderAPI_t::Vulkan)
		{
			// #TODO dynamic_cast bad!
			VulkanContext* vk_context = dynamic_cast<VulkanContext*>(m_context.get());
			vk_context->GetSwapchain().InitSurface(m_window);

			uint32_t width = m_data.Width, height = m_data.Height;
			vk_context->GetSwapchain().Create(&width, &height, m_data.VSync);
		}

		KB_CORE_INFO("Context created!");
        
        
        glfwSetWindowUserPointer(m_window, &m_data);
        SetVsync(false);

        //GLFW Callbacks
        glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height){
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            WindowResizeEvent event(width, height);
            data.EventCallback(event);
            data.Width = width;
            data.Height = height;

            if (width == 0 || height == 0)
            {
                WindowMinimizeEvent event;
                data.EventCallback(event);
            }
        });

        glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            WindowCloseEvent event;
            data.EventCallback(event);
            
        });

        glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action) {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event(key, 0);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(key);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(key, 1);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int keycode) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            KeyTypedEvent event(keycode);
            data.EventCallback(event);
        });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action) {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseScrolledEvent event((float)xOffset, (float)yOffset);
            data.EventCallback(event);
        });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseMovedEvent event((float)xPos, (float)yPos);
            data.EventCallback(event);
        });

		// Make sure window data about size is actual size
		{
			int width, height;
			glfwGetWindowSize(m_window, &width, &height);
			m_data.Width = width;
			m_data.Height = height;
		}

    }

    void WindowsWindow::Shutdown()
    {
        KB_PROFILE_FUNCTION();

		if (RendererAPI::GetAPI() == RendererAPI::RenderAPI_t::Vulkan)
		{
			// #TODO dynamic_cast bad!
			VulkanContext* vk_context = dynamic_cast<VulkanContext*>(m_context.get());
			vk_context->GetSwapchain().Destroy();
		}

		m_context->Shutdown();

        glfwDestroyWindow(m_window);
		--s_glfw_window_count;

		if (s_glfw_window_count == 0)
			glfwTerminate();
    }

	void WindowsWindow::PollEvents()
	{
		glfwPollEvents();
	}

    void WindowsWindow::OnUpdate()
    {
        KB_PROFILE_FUNCTION();

        m_context->SwapBuffers();
    }


    void WindowsWindow::SetVsync(bool enabled)
    {
		if (RendererAPI::GetAPI() == RendererAPI::RenderAPI_t::OpenGL)
		{
			if (enabled)
				glfwSwapInterval(1);
			else
				glfwSwapInterval(0);
		}
      
        m_data.VSync = enabled;
    }

    bool WindowsWindow::IsVsync() const
    {
        return m_data.VSync;
    }

	void WindowsWindow::SetWindowTitle(const std::string& title)
	{
        m_data.Title = title;
        glfwSetWindowTitle(m_window, m_data.Title.c_str());
	}

}
