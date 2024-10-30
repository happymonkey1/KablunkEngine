#include "kablunkpch.h"

#include "Kablunk/Core/Window.h"

#include <vulkan/vulkan.h>
#ifdef KB_PLATFORM_WINDOWS
#   include "Platform/Windows/WindowsWindow.h"
#endif



namespace kb
{ // start namespace kb

box<Window> Window::Create(
    render::backend::swap_chain* p_swap_chain_ptr,
    WindowProps p_props /* = {} */
)
{
#if defined(KB_PLATFORM_WINDOWS)
    return create_box<WindowsWindow>(p_props, p_swap_chain_ptr);
#elif defined(KB_PLATFORM_LINUX)
#   error linux_window is not implemented!
#elif defined(KB_PLATFORM_MAC)
#    error mac_window is not implemented!
#else
#   error unknown platform!
#endif
}

} // end namespace kb
