#include "kablunkpch.h"

#include "kablunk/renderer/shader_library.h"

namespace kb::render
{ // start namespace kb::render

auto shader_library::add(const std::string& name, const arc<backend::shader>& shader) noexcept -> const arc<backend::shader>&
{
    KB_CORE_ASSERT(!exists(name), "Shader already exists!");
    m_shaders[name] = shader;
    return m_shaders[name];
}

auto shader_library::add(const arc<backend::shader>& shader) noexcept -> const arc<backend::shader>&
{
    auto& name = shader->get_name();
    return add(name, shader);
}

const arc<backend::shader>& shader_library::load(const std::string& filepath)
{
    const auto shader{ backend::shader::create(filepath) };
    return add(shader);
}

const arc<backend::shader>& shader_library::load(const std::string& name, const std::string& filepath)
{
    const auto shader = backend::shader::create(filepath);
    return add(name, shader);
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
