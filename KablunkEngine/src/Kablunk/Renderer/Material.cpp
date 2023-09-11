#include "kablunkpch.h"

#include "Kablunk/Renderer/Material.h"
#include "Kablunk/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLMaterial.h"
#include "Platform/Vulkan/VulkanMaterial.h"

namespace Kablunk
{
	ref<Material> Material::Create(const ref<Shader>& shader, const std::string& name /* = "" */)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::OpenGL:	KB_CORE_ASSERT(false, "not implemented!"); return nullptr;//return IntrusiveRef<OpenGLMaterial>::Create(shader, name);
		case RendererAPI::render_api_t::Vulkan:	return ref<VulkanMaterial>::Create(shader, name);
		case RendererAPI::render_api_t::None:	KB_CORE_ASSERT(false, "No rendererAPI set!"); return nullptr;
		default:								KB_CORE_ASSERT(false, "Unknown rendererAPI set!"); return nullptr;
		}
	}

	ref<Material> Material::Copy(const ref<Material>& material, const std::string& name /* = "" */)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::render_api_t::OpenGL:  KB_CORE_ASSERT(false, "Not implemented!") return nullptr;
		case RendererAPI::render_api_t::Vulkan:  return ref<VulkanMaterial>::Create(material, name);
		case RendererAPI::render_api_t::None:	KB_CORE_ASSERT(false, "No rendererAPI set!"); return nullptr;
		default:								KB_CORE_ASSERT(false, "Unknown rendererAPI set!"); return nullptr;
		}
	}
}
