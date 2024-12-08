#include "kablunkpch.h"

#include "kablunk/renderer/shader_library.h"

namespace kb::render
{ // start namespace kb::render

void shader_library::add(const std::string& name, const arc<backend::shader>& shader)
{
    KB_CORE_ASSERT(!exists(name), "Shader already exists!");
    m_shaders[name] = shader;
}

void shader_library::add(const arc<backend::shader>& shader)
{
    auto& name = shader->get_name();
    add(name, shader);
}

const arc<backend::shader>& shader_library::load(const std::string& filepath)
{
    auto shader{ backend::shader::create(filepath) };
    add(shader);
    return shader;
}

const arc<backend::shader>& shader_library::load(const std::string& name, const std::string& filepath)
{
    auto shader = backend::shader::create(filepath);
    add(name, shader);
    return shader;
}

const arc<backend::shader>& shader_library::get(const std::string& name)
{
    KB_CORE_ASSERT(exists(name), "Shader does not exist!");
    return m_shaders[name];
}

bool shader_library::exists(const std::string& name)
{
    return m_shaders.find(name) != m_shaders.end();
}

} // end namespace kb::render
