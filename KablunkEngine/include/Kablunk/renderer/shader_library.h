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

    void Destroy();

    void Add(const arc<backend::shader>& shader);
    void Add(const std::string& name, const arc<backend::shader>& shader);
    arc<backend::shader> Load(const std::string& filepath);
    arc<backend::shader> Load(const std::string& name, const std::string& filepath);

    arc<backend::shader> Get(const std::string& name);

    bool Exists(const std::string& name);

private:
    unordered_flat_map<std::string, arc<backend::shader>> m_shaders;
};

} // end namespace kb::render
