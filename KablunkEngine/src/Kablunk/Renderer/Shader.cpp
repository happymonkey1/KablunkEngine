#include "kablunkpch.h"
#include "Kablunk/Renderer/Shader.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanShader.h"

namespace kb
{
arc<Shader> Shader::Create(const std::string& file_path, bool force_compile)
{
	switch (render::Renderer::get_render_backend_type())
	{
	case render::render_backend_type_t::vulkan:
        return static_cast<arc<Shader>>(arc<VulkanShader>::Create(file_path, force_compile));
	default:
        KB_CORE_ASSERT(
            false,
            "[Shader::Create]: Unhandled render backend {}!",
            static_cast<std::underlying_type_t<render::render_backend_type_t>>(render::Renderer::get_render_backend_type())
        );
	    return arc<Shader>{};
	}
}

ShaderLibrary::~ShaderLibrary()
{
}

void ShaderLibrary::Destroy()
{
	KB_CORE_INFO("Destroying shader library!");

	for (auto& [shader_name, shader] : m_shaders)
		shader->destroy();

	m_shaders.clear();
}

void ShaderLibrary::Add(const std::string& name, const arc<Shader>& shader)
{
	KB_CORE_ASSERT(!Exists(name), "Shader already exists!");
	m_shaders[name] = shader;
}

void ShaderLibrary::Add(const arc<Shader>& shader)
{
	auto& name = shader->GetName();
	Add(name, shader);
}

arc<Shader> ShaderLibrary::Load(const std::string& filepath)
{
    auto shader{ Shader::Create(filepath) };
	Add(shader);
	return shader;
}

arc<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath)
{
	auto shader = Shader::Create(filepath);
	Add(name, shader);
	return shader;
}

arc<kb::Shader> ShaderLibrary::Get(const std::string& name)
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
	if (type == ShaderUniformType::Int)
		return std::string{ "Int" };
	if (type == ShaderUniformType::Float)
		return std::string{ "Float" };

	return std::string{ "None" };
}
}
