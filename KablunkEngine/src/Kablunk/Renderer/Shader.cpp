#include "kablunkpch.h"
#include "Kablunk/Renderer/Shader.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace Kablunk
{
	std::vector<IntrusiveRef<Shader>> Shader::s_all_shaders;

	IntrusiveRef<Shader> Shader::Create(const std::string& file_path, bool force_compile)
	{
		IntrusiveRef<Shader> res = nullptr;
		switch (Renderer::GetAPI())
		{
		case RendererAPI::RenderAPI_t::None:		KB_CORE_ASSERT(false, "RendererAPI::None is not supported when creating Shader!"); return nullptr;
		case RendererAPI::RenderAPI_t::OpenGL:		res = IntrusiveRef<OpenGLShader>::Create(file_path); break;
		case RendererAPI::RenderAPI_t::Vulkan:		res = IntrusiveRef<VulkanShader>::Create(file_path, force_compile); break;
		default:									KB_CORE_ASSERT(false, "Unkown RenderAPI!"); return nullptr;
		}

		s_all_shaders.push_back(res);
		return res;
	}

	ShaderLibrary::~ShaderLibrary()
	{
		Destroy();
	}

	void ShaderLibrary::Destroy()
	{
		m_shaders.clear();
	}

	void ShaderLibrary::Add(const std::string& name, const IntrusiveRef<Shader>& shader)
	{
		KB_CORE_ASSERT(!Exists(name), "Shader already exists!");
		m_shaders[name] = shader;
	}

	void ShaderLibrary::Add(const IntrusiveRef<Shader>& shader)
	{
		auto& name = shader->GetName();
		Add(name, shader);
	}

	
	IntrusiveRef<Shader> ShaderLibrary::Load(const std::string& filepath)
	{
		auto& shader = Shader::Create(filepath);
		Add(shader);
		return shader;
	}

	IntrusiveRef<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath)
	{
		auto shader = Shader::Create(filepath);
		Add(name, shader);
		return shader;
	}

	IntrusiveRef<Kablunk::Shader> ShaderLibrary::Get(const std::string& name)
	{
		KB_CORE_ASSERT(Exists(name), "Shader does not exist!");
		return m_shaders[name];
	}

	bool ShaderLibrary::Exists(const std::string& name)
	{
		return m_shaders.find(name) != m_shaders.end();
	}

	ShaderUniform::ShaderUniform(std::string name, ShaderUniformType type, uint32_t size, uint32_t offset)
		: m_name{ std::move(name) }, m_type{ type }, m_size{ size }, m_offset{ offset }
	{

	}

	std::string ShaderUniform::UniformTypeToString(ShaderUniformType type)
	{
		if (type == ShaderUniformType::Bool)
			return std::string{ "Boolean" };
		else if (type == ShaderUniformType::Int)
			return std::string{ "Int" };
		else if (type == ShaderUniformType::Float)
			return std::string{ "Float" };

		return std::string{ "None" };
	}


}
