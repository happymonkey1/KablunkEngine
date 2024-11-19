#include "kablunkpch.h"

#include "kablunk/renderer/shader_library.h"

namespace kb::render
{ // start namespace kb::render

void shader_library::Destroy()
{
    KB_CORE_INFO("Destroying shader library!");

    for (auto& [shader_name, shader] : m_shaders)
        shader->destroy();

    m_shaders.clear();
}

void shader_library::Add(const std::string& name, const arc<backend::shader>& shader)
{
    KB_CORE_ASSERT(!Exists(name), "Shader already exists!");
    m_shaders[name] = shader;
}

void shader_library::Add(const arc<backend::shader>& shader)
{
    auto& name = shader->get_name();
    Add(name, shader);
}

arc<backend::shader> shader_library::Load(const std::string& filepath)
{
    auto shader{ backend::shader::create(filepath) };
    Add(shader);
    return shader;
}

arc<backend::shader> shader_library::Load(const std::string& name, const std::string& filepath)
{
    auto shader = backend::shader::create(filepath);
    Add(name, shader);
    return shader;
}

arc<backend::shader> shader_library::Get(const std::string& name)
{
    KB_CORE_ASSERT(Exists(name), "Shader does not exist!");
    return m_shaders[name];
}

bool shader_library::Exists(const std::string& name)
{
    return m_shaders.find(name) != m_shaders.end();
}

} // end namespace kb::render
