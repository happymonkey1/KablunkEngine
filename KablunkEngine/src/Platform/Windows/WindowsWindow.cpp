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
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        
        KB_CORE_INFO("Creating Window {0} ({1} {2})", props.Title, props.Width, props.Height);
        

        if (s_glfw_window_count == 0) 
		{
			KB_PROFILE_SCOPE("glfwInit");

			int success = glfwInit();
            KB_CORE_ASSERT(success, "COULD NOT INITIALIZE GLFW");

			// Hint to glfw that this will be rendered with Vulkan
			if (RendererAPI::GetAPI() == RendererAPI::RenderAPI_t::Vulkan)
			{
				glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
				glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
			}
			

            glfwSetErrorCallback(GLFWErrorCallback);
        }

        {
			KB_PROFILE_SCOPE("glfwCreateWindow");

            m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
			++s_glfw_window_count;
        }
		m_Context = GraphicsContext::Create(m_Window);
		m_Context->Init();

		if (RendererAPI::GetAPI() == RendererAPI::RenderAPI_t::Vulkan)
		{
			// #TODO bad!
			VulkanContext* vk_context = dynamic_cast<VulkanContext*>(m_Context.get());
			vk_context->GetSwapchain().InitSurface(m_Window);

			uint32_t width = m_Data.Width, height = m_Data.Height;
			vk_context->GetSwapchain().Create(&width, &height, m_Data.VSync);
		}

		KB_CORE_INFO("Context created!");
        
        
        glfwSetWindowUserPointer(m_Window, &m_Data);
        SetVsync(false);

        //GLFW Callbacks
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height){
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

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            WindowCloseEvent event;
            data.EventCallback(event);
            
        });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
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

        glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            KeyTypedEvent event(keycode);
            data.EventCallback(event);
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
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

        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseScrolledEvent event((float)xOffset, (float)yOffset);
            data.EventCallback(event);
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            MouseMovedEvent event((float)xPos, (float)yPos);
            data.EventCallback(event);
        });

		// Make sure window data about size is actual size
		{
			int width, height;
			glfwGetWindowSize(m_Window, &width, &height);
			m_Data.Width = width;
			m_Data.Height = height;
		}

    }
    void WindowsWindow::Shutdown()
    {
        KB_PROFILE_FUNCTION();

		m_Context->Shutdown();

        glfwDestroyWindow(m_Window);
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

        m_Context->SwapBuffers();
    }


    void WindowsWindow::SetVsync(bool enabled)
    {
        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);

        m_Data.VSync = enabled;
    }

    bool WindowsWindow::IsVsync() const
    {
        return m_Data.VSync;
    }

	void WindowsWindow::SetWindowTitle(const std::string& title)
	{
        m_Data.Title = title;
        glfwSetWindowTitle(m_Window, m_Data.Title.c_str());
	}

}
