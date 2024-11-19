#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_SHADER_UNIFORM_H
#define KABLUNK_RENDERER_BACKEND_SHADER_UNIFORM_H

#include <Kablunk/Core/CoreTypes.h>

#include <string>
#include <vector>

namespace kb::render::backend
{ // start namespace kb::render::backend

enum class shader_domain_t
{
	None = 0, Vertex = 0, Pixel = 1 // unused
};

class shader_resource_declaration
{
public:
	shader_resource_declaration() = default;
    ~shader_resource_declaration() = default;

	shader_resource_declaration(
        std::string p_name,
        u32 p_set,
        u32 p_resource_register,
        u32 p_count
    )
        : m_name{ p_name }, m_descriptor_set{ p_set }, m_register{ p_resource_register }, m_count{ p_count }
    {
    }

	virtual const std::string& get_name() const noexcept { return m_name; }
	virtual u32 get_register() const noexcept { return m_register; }
	virtual u32 get_count() const noexcept { return m_count; }
    virtual u32 get_set() const noexcept { return m_descriptor_set; }

private:
	std::string m_name;
    u32 m_descriptor_set = 0;
	u32 m_register = 0;
	u32 m_count = 0;
};


} // end namespace kb::render::backend

#endif
