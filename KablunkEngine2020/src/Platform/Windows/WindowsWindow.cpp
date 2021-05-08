#include "kablunkpch.h"
#include "Platform/Windows/WindowsWindow.h"


#include "Kablunk/Events/KeyEvent.h"
#include "Kablunk/Events/MouseEvent.h"
#include "Kablunk/Events/ApplicationEvent.h"

#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/OpenGL/OpenGLContext.h"
#include "GLFW/glfw3.h"



namespace Kablunk {

    static bool s_GLFWInitialized = false;

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
        

        if (!s_GLFWInitialized) {
            KB_PROFILE_SCOPE("glfwInit")

            KB_CORE_ASSERT(glfwInit(), "COULD NOT INITIALIZE GLFW");

            glfwSetErrorCallback(GLFWErrorCallback);
            s_GLFWInitialized = true;
        }

        {
            KB_PROFILE_SCOPE("glfwCreateWindow")
            m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), NULL, NULL);
        }
		m_Context = CreateScope<OpenGLContext>(m_Window);
		m_Context->Init();
		KB_CORE_INFO("Context created!");
        
        
        glfwSetWindowUserPointer(m_Window, &m_Data);
        SetVsync(true);

        //GLFW Callbacks
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height){
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            WindowResizeEvent event(width, height);
            data.EventCallback(event);
            data.Width = width;
            data.Height = height;

            
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
    }
    void WindowsWindow::Shutdown()
    {
        KB_PROFILE_FUNCTION();

        glfwDestroyWindow(m_Window);

        glfwTerminate();
    }

    void WindowsWindow::OnUpdate()
    {
        KB_PROFILE_FUNCTION();

        glfwPollEvents();
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