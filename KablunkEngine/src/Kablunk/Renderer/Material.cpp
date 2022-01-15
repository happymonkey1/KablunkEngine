#include "kablunkpch.h"

#include "Kablunk/Renderer/Material.h"
#include "Kablunk/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLMaterial.h"
#include "Platform/Vulkan/VulkanMaterial.h"

namespace Kablunk
{
	IntrusiveRef<Material> Material::Create(const IntrusiveRef<Shader>& shader, const std::string& name /* = "" */)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::RenderAPI_t::OpenGL:	KB_CORE_ASSERT(false, "not implemented!"); return nullptr;//return IntrusiveRef<OpenGLMaterial>::Create(shader, name);
		case RendererAPI::RenderAPI_t::Vulkan:	return IntrusiveRef<VulkanMaterial>::Create(shader, name);
		case RendererAPI::RenderAPI_t::None:	KB_CORE_ASSERT(false, "No rendererAPI set!"); return nullptr;
		default:								KB_CORE_ASSERT(false, "Unknown rendererAPI set!"); return nullptr;
		}
	}

	IntrusiveRef<Material> Material::Copy(const IntrusiveRef<Material>& material, const std::string& name /* = "" */)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::RenderAPI_t::OpenGL:  KB_CORE_ASSERT(false, "Not implemented!") return nullptr;
		case RendererAPI::RenderAPI_t::Vulkan:  KB_CORE_ASSERT(false, "Not implemented!") return nullptr;
		case RendererAPI::RenderAPI_t::None:	KB_CORE_ASSERT(false, "No rendererAPI set!"); return nullptr;
		default:								KB_CORE_ASSERT(false, "Unknown rendererAPI set!"); return nullptr;
		}
	}
}
