#include "kablunkpch.h"
#include "WindowsWindow.h"

namespace kablunk {

    static bool s_GLFWInitialized = false;

    Window* Window::Create(const WindowProps& props) {
        return new WindowsWindow(props);
    }

    WindowsWindow::WindowsWindow(const WindowProps& props)
    {
        Init(props);
    }

    WindowsWindow::~WindowsWindow()
    {
    }

    void WindowsWindow::Init(const WindowProps& props)
    {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        KABLUNK_CORE_INFO("Creating Window {0} ({1} {2})", props.Title, props.Width, props.Height);

        if (!s_GLFWInitialized) {
            int success = glfwInit();
            if (!success)
                KABLUNK_CORE_ERROR("COULD NOT INITIALIZE GLFW");

            s_GLFWInitialized = true;
        }

        m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), NULL, NULL);
        glfwMakeContextCurrent(m_Window);
        glfwSetWindowUserPointer(m_Window, &m_Data);
        SetVsync(true);
    }

    void WindowsWindow::Shutdown()
    {
        KABLUNK_CLIENT_WARN("NEED TO IMPLEMENT WINDOWS WINDOW SHUTDOWN");
    }

    void WindowsWindow::OnUpdate()
    {
        glfwPollEvents();
        glfwSwapBuffers(m_Window);
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

    

    
}