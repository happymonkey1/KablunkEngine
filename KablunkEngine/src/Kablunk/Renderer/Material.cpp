#include "kablunkpch.h"

#include "Kablunk/Renderer/Material.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/RendererAPI.h"
#include "Platform/Vulkan/VulkanMaterial.h"

namespace kb
{ // start namespace kb
arc<Material> Material::Create(const arc<Shader>& shader, const std::string& name /* = "" */)
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    switch (backend)
    {
    case render::render_backend_type_t::vulkan:
        return static_cast<arc<Material>>(arc<VulkanMaterial>::Create(shader, name));
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return arc<Material>{};
    }
    }
}

arc<Material> Material::Copy(const arc<Material>& material, const std::string& name /* = "" */)
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    switch (backend)
    {
    case render::render_backend_type_t::vulkan:
        return static_cast<arc<Material>>(arc<VulkanMaterial>::Create(material, name));
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return arc<Material>{};
    }
    }
}
} // end namespace kb
