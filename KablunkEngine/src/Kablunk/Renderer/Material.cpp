#include "kablunkpch.h"

#include "Kablunk/Renderer/Material.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Renderer/RendererAPI.h"
#include "Platform/Vulkan/VulkanMaterial.h"

namespace kb
{ // start namespace kb
ref<Material> Material::Create(const ref<Shader>& shader, const std::string& name /* = "" */)
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    switch (backend)
    {
    case render::render_backend_type_t::vulkan:
        return static_cast<ref<Material>>(ref<VulkanMaterial>::Create(shader, name));
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return ref<Material>{};
    }
    }
}

ref<Material> Material::Copy(const ref<Material>& material, const std::string& name /* = "" */)
{
    constexpr auto backend = render::Renderer::get_render_backend_type();
    switch (backend)
    {
    case render::render_backend_type_t::vulkan:
        return static_cast<ref<Material>>(ref<VulkanMaterial>::Create(material, name));
    default:
    {
        KB_CORE_ASSERT(false, "Unhandled render backend type!"); return ref<Material>{};
    }
    }
}
} // end namespace kb
