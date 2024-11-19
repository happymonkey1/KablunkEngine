#include "kablunkpch.h"

#include "Kablunk/renderer/backend/material.h"

#include "Kablunk/renderer/renderer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_material.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

arc<material> material::create(const arc<shader>& shader, const std::string& name /* = "" */)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    switch (backend)
    {
    case render_backend_type_t::vulkan:
        return static_cast<arc<material>>(arc<vk::vulkan_material>::Create(shader, name));
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return arc<material>{};
    }
    }
}

arc<material> material::copy(const arc<material>& p_material, const std::string& p_name /* = "" */)
{
    constexpr auto backend = Renderer::get_render_backend_type();
    switch (backend)
    {
    case render_backend_type_t::vulkan:
        return static_cast<arc<material>>(arc<vk::vulkan_material>::Create(p_material, p_name));
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return arc<material>{};
    }
    }
}

} // end namespace kb::render::backend
