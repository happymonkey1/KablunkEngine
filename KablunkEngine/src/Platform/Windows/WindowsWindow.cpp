#include "kablunkpch.h"
#include "Platform/Windows/WindowsWindow.h"


#include "Kablunk/Events/KeyEvent.h"
#include "Kablunk/Events/MouseEvent.h"
#include "Kablunk/Events/ApplicationEvent.h"

#include "Kablunk/Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanContext.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace kb
{ // start namespace kb

static uint8_t s_glfw_window_count = 0;

static void GLFWErrorCallback(int error, const char* desc) 
{
    KB_CORE_ERROR("GLFW Error ({0} {1})", error, desc);
}

box<Window> Window::Create(const WindowProps& props) 
{
    return create_box<WindowsWindow>(props);
}

WindowsWindow::WindowsWindow(const WindowProps& props)
{
    KB_PROFILE_SCOPE;

    Init(props);
}

WindowsWindow::~WindowsWindow()
{
    KB_PROFILE_SCOPE;

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
		int success = glfwInit();
        KB_CORE_ASSERT(success, "COULD NOT INITIALIZE GLFW");

		// Hint to glfw that this will be rendered with Vulkan
		if (RendererAPI::GetAPI() == RendererAPI::render_api_t::Vulkan)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			// #TODO resizing at runtime
			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		}

        glfwSetErrorCallback(GLFWErrorCallback);
    }

	{
		GLFWmonitor* primary_monitor = nullptr;
		if (m_data.Fullscreen)
		{
			// #NOTE(Sean) don't set primary monitor and disable decorations to get bordless fullscreen windowed mode
			//primary_monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

			glfwWindowHint(GLFW_RED_BITS, mode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

			m_data.Width = mode->width;
			m_data.Height = mode->height;

			// hint for fullscreen borderless
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		}

		m_window = glfwCreateWindow((int)m_data.Width, (int)m_data.Height, m_data.Title.c_str(), primary_monitor, nullptr);
		++s_glfw_window_count;

        // get monitor work size and compute dpi manually
        {
            int x, y, resolution_width, resolution_height;
            int monitor_width = 0, monitor_height = 0;
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            // width and height reported in mm
            glfwGetMonitorWorkarea(monitor, &x, &y, &resolution_width, &resolution_height);
            glfwGetMonitorPhysicalSize(monitor, &monitor_width, &monitor_height);

            if (monitor_width > 0 && monitor_height > 0)
            {
                m_data.m_current_dpi = glm::round(compute_dpi(
                    glm::vec2{ resolution_width, resolution_height },
                    glm::vec2{ monitor_width, monitor_height }
                ));
            }
            else
            {
                m_data.m_current_dpi = glm::vec2{ 96.f, 96.f };
            }

            KB_CORE_INFO("[WindowsWindow]: GLFW reporting monitor DPI as ({}, {})", m_data.m_current_dpi.x, m_data.m_current_dpi.y);
        }
	}
	m_context = GraphicsContext::Create(m_window);
	m_context->Init();

	if (RendererAPI::GetAPI() == RendererAPI::render_api_t::Vulkan)
	{
		// #TODO dynamic_cast bad!
		ref<VulkanContext> context = m_context.As<VulkanContext>();
		//vk_context->GetSwapchain().Init(vk_context->GetInstance(), vk_context->GetDevice());
		context->GetSwapchain().InitSurface(m_window);

		uint32_t width = m_data.Width, height = m_data.Height;
		context->GetSwapchain().Create(&width, &height, m_data.VSync);
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

    // #TODO callback for dpi change

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
    KB_PROFILE_SCOPE;

	
	if (RendererAPI::GetAPI() == RendererAPI::render_api_t::Vulkan)
	{
		// #TODO dynamic_cast bad!
		VulkanContext* vk_context = dynamic_cast<VulkanContext*>(m_context.get());

		vk_context->GetSwapchain().Destroy();
        vk_context->GetDevice()->Destroy();
	}

	//m_context->Shutdown();

    glfwDestroyWindow(m_window);
	--s_glfw_window_count;

	if (s_glfw_window_count == 0)
		glfwTerminate();
}

auto WindowsWindow::compute_dpi(
    const glm::vec2& p_monitor_resolution,
    const glm::vec2& p_monitor_dimensions
) noexcept -> glm::vec2
{
    constexpr f32 k_mm_to_in = 0.0393701f;
    return glm::vec2{
        static_cast<f32>(p_monitor_resolution.x) / (static_cast<f32>(p_monitor_dimensions.x) * k_mm_to_in),
        static_cast<f32>(p_monitor_resolution.y) / (static_cast<f32>(p_monitor_dimensions.y) * k_mm_to_in),
    };
}

void WindowsWindow::PollEvents()
{
    KB_PROFILE_SCOPE;

	glfwPollEvents();
}

void WindowsWindow::OnUpdate()
{
    KB_PROFILE_SCOPE;

    m_context->SwapBuffers();
}


const glm::vec2& WindowsWindow::get_current_dpi() const noexcept
{
    return m_data.m_current_dpi;
}

void WindowsWindow::SetVsync(bool enabled)
{
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

void WindowsWindow::set_window_mode(window_mode_t mode)
{
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* glfw_mode = glfwGetVideoMode(monitor);
	KB_CORE_ASSERT(glfw_mode, "null glfw mode?");
	// whether or not to enable decoration bar
	int decoration_value = GLFW_FALSE;
	// starting position of the window
	int x_pos = 0, y_pos = 0;
	// refresh rate of the primary monitor
	int refresh_rate = glfw_mode->refreshRate;
	switch (mode)
	{
		case window_mode_t::windowed:
		{
			// enable decorations (top bar of windowed and fullscreen mode)
			decoration_value = GLFW_TRUE;

			// #TODO(Sean) use cached, previous window size
			m_data.Width = 1920;
			m_data.Height = 1080;
			m_data.Fullscreen = false;

			x_pos = m_data.Width / 2;
			y_pos = m_data.Height / 2;

			break;
		}
		case window_mode_t::fullscreen:
		{
			decoration_value = GLFW_TRUE;

			glfwWindowHint(GLFW_RED_BITS, glfw_mode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, glfw_mode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, glfw_mode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, glfw_mode->refreshRate);

			m_data.Width = glfw_mode->width;
			m_data.Height = glfw_mode->height;
			m_data.Fullscreen = true;

			break;
		}
		case window_mode_t::borderless_fullscreen:
		{
			decoration_value = GLFW_FALSE;

			glfwWindowHint(GLFW_RED_BITS, glfw_mode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, glfw_mode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, glfw_mode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, glfw_mode->refreshRate);

			m_data.Width = glfw_mode->width;
			m_data.Height = glfw_mode->height;
			m_data.Fullscreen = true;

			// disable decorations (top bar of windowed and fullscreen mode)
			//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
			break;
		}
		default:
			KB_CORE_ASSERT(false, "unhandled window mode!");
	}

	// #TODO figure out why glfw mode is returning "incorrect" values

	// enable decorations (top bar of windowed and fullscreen mode)
	glfwSetWindowAttrib(m_window, GLFW_DECORATED, decoration_value);

	glfwSetWindowMonitor(m_window, m_data.Fullscreen ? monitor : nullptr, x_pos, y_pos, m_data.Width, m_data.Height, refresh_rate);
}

void WindowsWindow::swap_buffers()
{
	// #TODO this is not renderer agnostic
	VulkanContext::Get()->GetSwapchain().Present();
}

cursor_handle WindowsWindow::create_cursor(
    ref<Texture2D>& p_texture,
    const glm::ivec2& p_hot_spot
) noexcept
{
    KB_CORE_ASSERT(p_texture, "[WindowsWindow]: Trying to load cursor but texture is null?");

    KB_CORE_ASSERT(
        m_cursor_count < k_max_cursors,
        "[WindowsWindow]: Max concurrent cursor count {} reached!",
        k_max_cursors
    );

    GLFWimage image{
        .width = static_cast<i32>(p_texture->GetWidth()),
        .height = static_cast<i32>(p_texture->GetHeight()),
        .pixels = static_cast<unsigned char*>(p_texture->GetWriteableBuffer().get())
    };

    const auto glfw_cursor = glfwCreateCursor(&image, p_hot_spot.x, p_hot_spot.y);
    const auto new_cursor_handle = cursor_handle{ static_cast<u8>(m_cursor_count) };
    m_cursors[m_cursor_count++] = glfw_cursor;

    return new_cursor_handle;
}

void WindowsWindow::set_cursor(cursor_handle p_cursor_handle) noexcept
{
    KB_CORE_ASSERT(
        static_cast<size_t>(static_cast<cursor_handle::value_t>(p_cursor_handle)) < m_cursor_count,
        "[WindowsWindow]: Cursor index is out of buffer range!"
    );

    glfwSetCursor(m_window, m_cursors[static_cast<cursor_handle::value_t>(p_cursor_handle)]);
}

void WindowsWindow::set_default_cursor() noexcept
{
    glfwSetCursor(m_window, nullptr);
}

} // end namespace kb
