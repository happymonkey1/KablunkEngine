#pragma once

#include "Kablunk/Core/RefCounting.h"
#include "kablunk/renderer/backend/shader.h"

namespace kb::render
{ // start namespace kb::render

class shader_library : public RefCounted
{
public:
    shader_library() = default;
    ~shader_library() noexcept override = default;

    auto add(const arc<backend::shader>& shader) noexcept -> const arc<backend::shader>&;
    auto add(const std::string& name, const arc<backend::shader>& shader) noexcept -> const arc<backend::shader>&;
    const arc<backend::shader>& load(const std::string& filepath);
    const arc<backend::shader>& load(const std::string& name, const std::string& filepath);

    const arc<backend::shader>& get(const std::string& name);

    bool exists(const std::string& name);

private:
    unordered_flat_map<std::string, arc<backend::shader>> m_shaders;
};

} // end namespace kb::render
