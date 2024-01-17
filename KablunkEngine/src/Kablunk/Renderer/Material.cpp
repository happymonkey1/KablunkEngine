#include "kablunkpch.h"

#include "Kablunk/Renderer/Material.h"
#include "Kablunk/Renderer/RendererAPI.h"
#include "Platform/Vulkan/VulkanMaterial.h"

namespace kb
{
	ref<Material> Material::Create(const ref<Shader>& shader, const std::string& name /* = "" */)
	{
		switch (RendererAPI::GetAPI())
		{
        case RendererAPI::render_api_t::Vulkan:	return static_cast<ref<Material>>(ref<VulkanMaterial>::Create(shader, name));
		case RendererAPI::render_api_t::None:	KB_CORE_ASSERT(false, "No rendererAPI set!"); return ref<Material>{};
		default:								KB_CORE_ASSERT(false, "Unknown rendererAPI set!"); return ref<Material>{};
		}
	}

	ref<Material> Material::Copy(const ref<Material>& material, const std::string& name /* = "" */)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::Vulkan:  return static_cast<ref<Material>>(ref<VulkanMaterial>::Create(material, name));
		case RendererAPI::render_api_t::None:	KB_CORE_ASSERT(false, "No rendererAPI set!"); return ref<Material>{};
		default:								KB_CORE_ASSERT(false, "Unknown rendererAPI set!"); return ref<Material>{};
		}
	}
}
