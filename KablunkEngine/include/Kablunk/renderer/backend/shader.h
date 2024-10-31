#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/RefCounting.h"
#include "kablunk/renderer/renderer_types.h"
#include "Kablunk/Renderer/backend/shader_resource_declaration.h"

#include <glm/glm.hpp>

#include <string>

namespace kb::render::backend
{ // start namespace kb::render::backend

enum class shader_uniform_type_t
{
	None = 0, Bool, Int, UInt, Float, Vec2, Vec3, Vec4, Mat3, Mat4,
	IVec2, IVec3, IVec4
};

class shader_uniform
{
public:
	shader_uniform() = default;
	shader_uniform(std::string name, shader_uniform_type_t type, u32 size, u32 offset);

	const std::string& get_name() const { return m_name; }
	shader_uniform_type_t get_type() const { return m_type; }
    u32 get_size() const { return m_size; }
    u32 get_offset() const { return m_offset; }

	static std::string get_uniform_type_to_string(shader_uniform_type_t type);
private:
	std::string m_name;
	shader_uniform_type_t m_type = shader_uniform_type_t::None;
    u32 m_size = 0;
    u32 m_offset = 0;
};

struct shader_uniform_buffer_t
{
	std::string name;
    u32 index;
    u32 binding_point;
    u32 size;
    u32 renderer_ID;
	std::vector<shader_uniform> uniforms;
};

struct shader_storage_buffer_t
{
	std::string name;
    u32 index;
    u32 binding_point;
    u32 size;
    u32 renderer_ID;
	//std::vector<ShaderUniform> Uniforms;
};

struct shader_buffer_t
{
	std::string name;
    u32 size = 0;
	unordered_flat_map<std::string, shader_uniform> uniforms;
};


class shader : public RefCounted
{
public:
	using ShaderReloadedCallback = std::function<void()>;
    ~shader() override = default;

	// destroy shader and free resources
	virtual void destroy() = 0;

	virtual void reload(bool force_compile = false) = 0;
	virtual size_t get_hash() const = 0;

	virtual void add_shader_reloaded_callback(const ShaderReloadedCallback& callback) = 0;

	virtual void bind() const = 0;
	virtual void unbind() const = 0;

	virtual void set_mat4(const std::string& name, const glm::mat4& value) = 0;
	virtual void set_float(const std::string& name, float value) = 0;
	virtual void set_float2(const std::string& name, const glm::vec2& value) = 0;
	virtual void set_float3(const std::string& name, const glm::vec3& value) = 0;
	virtual void set_float4(const std::string& name, const glm::vec4& value) = 0;
	virtual void set_int(const std::string& name, int value) = 0;
	virtual void set_int_array(const std::string& name, int* values, u32 count) = 0;

	virtual const std::string& get_name() const = 0;
	virtual RendererID get_renderer_id() const = 0;

	virtual const kb::unordered_flat_map<std::string, shader_buffer_t>& get_shader_buffers() const = 0;
	virtual const kb::unordered_flat_map<std::string, shader_resource_declaration>& get_resources() const = 0;

	static arc<shader> create(const std::string& p_file_path, bool p_force_compile = false);
};

} // end namespace kb::render::backend
