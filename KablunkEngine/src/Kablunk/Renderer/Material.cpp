#include "kablunkpch.h"

#include "Kablunk/Renderer/Material.h"
#include "Kablunk/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLMaterial.h"


namespace Kablunk
{
	Ref<Material> Material::Create(const Ref<Shader>& shader, const std::string& name /* = "" */)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::RenderAPI_t::OpenGL:	return CreateRef<OpenGLMaterial>(shader, name);
		case RendererAPI::RenderAPI_t::None:	KB_CORE_ASSERT(false, "No rendererAPI set!"); return nullptr;
		default:								KB_CORE_ASSERT(false, "Unknown rendererAPI set!"); return nullptr;
		}
	}

	Ref<Material> Material::Copy(const Ref<Material>& material, const std::string& name /* = "" */)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::RenderAPI_t::OpenGL:  KB_CORE_ASSERT(false, "Not implemented!") return nullptr;
		case RendererAPI::RenderAPI_t::None:	KB_CORE_ASSERT(false, "No rendererAPI set!"); return nullptr;
		default:								KB_CORE_ASSERT(false, "Unknown rendererAPI set!"); return nullptr;
		}
	}
}
