#ifndef KABLUNK_RENDERER_MATERIAL_H
#define KABLUNK_RENDERER_MATERIAL_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/backend/shader.h"
#include "Kablunk/Renderer/backend/texture.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

enum class MaterialFlag
{
	None		= BIT(0),
	DepthTest	= BIT(1),
	Blend		= BIT(2),
	TwoSided	= BIT(3)
};


class material : public RefCounted
{
public:
	~material() override = default;

	virtual void invalidate() = 0;
	virtual void bind() = 0;

	virtual void set(const std::string& name, float value) = 0;
	virtual void set(const std::string& name, int value) = 0;
	virtual void set(const std::string& name, uint32_t value) = 0;
	virtual void set(const std::string& name, bool value) = 0;
	virtual void set(const std::string& name, const glm::vec2& value) = 0;
	virtual void set(const std::string& name, const glm::vec3& value) = 0;
	virtual void set(const std::string& name, const glm::vec4& value) = 0;
	virtual void set(const std::string& name, const glm::ivec2& value) = 0;
	virtual void set(const std::string& name, const glm::ivec3& value) = 0;
	virtual void set(const std::string& name, const glm::ivec4& value) = 0;
	virtual void set(const std::string& name, const glm::mat3& value) = 0;
	virtual void set(const std::string& name, const glm::mat4& value) = 0;
	virtual void set(const std::string& name, const arc<texture_2d>& texture) = 0;
	virtual void set(const std::string& name, const arc<texture_2d>& texture, uint32_t array_index) = 0;
	virtual void set(const std::string& name, const arc<image_2d>& texture) = 0;

	virtual bool& get_bool(const std::string& name) = 0;
	virtual float& get_float(const std::string& name) = 0;
	virtual int& get_int(const std::string& name) = 0;
	virtual uint32_t& get_uint(const std::string& name) = 0;
	virtual glm::vec2& get_vec2(const std::string& name) = 0;
	virtual glm::vec3& get_vec3(const std::string& name) = 0;
	virtual glm::vec4& get_vec4(const std::string& name) = 0;
	virtual glm::mat3& get_mat3(const std::string& name) = 0;
	virtual glm::mat4& get_mat4(const std::string& name) = 0;
	virtual arc<texture_2d> get_texture_2d(const std::string& name) = 0;
	virtual arc<texture_2d> try_get_texture_2d(const std::string& name) = 0;

	virtual arc<shader> get_shader() = 0;
	virtual const std::string& get_name() const = 0;

	virtual uint32_t get_flags() const = 0;
	virtual bool get_flag(MaterialFlag flag) const = 0;
	virtual void set_flag(MaterialFlag flag, bool value = true) = 0;

	static arc<material> create(const arc<shader>& shader, const std::string& name = "");
	static arc<material> copy(const arc<material>& p_material, const std::string& p_name = "");
};

} // end namespace kb::render::backend

#endif
