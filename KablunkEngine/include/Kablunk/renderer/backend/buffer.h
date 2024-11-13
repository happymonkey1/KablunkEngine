#ifndef KABLUNK_RENDERER_BACKEND_BUFFER_H
#define KABLUNK_RENDERER_BACKEND_BUFFER_H

#include "Kablunk/Core/Core.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

enum class vertex_buffer_usage_t
{
	Static = 0, Dynamic
};

enum class shader_data_type_t
{
	None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
};

static uint32_t get_shader_data_type_size(shader_data_type_t type)
{
	switch (type)
	{
	case shader_data_type_t::Float:    return 4;
	case shader_data_type_t::Float2:   return 2 * 4;
	case shader_data_type_t::Float3:   return 3 * 4;
	case shader_data_type_t::Float4:   return 4 * 4;
	case shader_data_type_t::Mat3:     return 3*3*3;
	case shader_data_type_t::Mat4:     return 4*4*4;
	case shader_data_type_t::Int:      return 4;
	case shader_data_type_t::Int2:     return 2 * 4;
	case shader_data_type_t::Int3:     return 3 * 4;
	case shader_data_type_t::Int4:     return 4 * 4;
	case shader_data_type_t::Bool:     return 1;
	default:     KB_CORE_ERROR("Unknown ShaderDataType!"); return 0;
	}
}

struct buffer_element
{
	std::string Name;
	shader_data_type_t Type;
	uint32_t Size;
	size_t Offset;
	bool Normalized;

	buffer_element() = default;

	buffer_element(shader_data_type_t type, const std::string& name, bool normalized = false)
		: Name{ name }, Type{ type }, Size{ get_shader_data_type_size(type) }, Offset{ 0 }, Normalized{ normalized }
	{
	}

	uint32_t GetComponentCount() const
	{
		switch (Type)
		{
		case shader_data_type_t::Float:    return 1;
		case shader_data_type_t::Float2:   return 2;
		case shader_data_type_t::Float3:   return 3;
		case shader_data_type_t::Float4:   return 4;
		case shader_data_type_t::Mat3:     return 3 * 3;
		case shader_data_type_t::Mat4:     return 4 * 4;
		case shader_data_type_t::Int:      return 1;
		case shader_data_type_t::Int2:     return 2;
		case shader_data_type_t::Int3:     return 3;
		case shader_data_type_t::Int4:     return 4;
		case shader_data_type_t::Bool:     return 1;
		default:     KB_CORE_ERROR("Unknown ShaderDataType!"); return 0;
		}
	}
};

class buffer_layout
{
public:
    buffer_layout() = default;

	buffer_layout(const std::initializer_list<buffer_element>& elements) 
		: m_elements{ elements }
	{
		calculate_offsets_and_stride();
	}

    ~buffer_layout() noexcept = default;

	const std::vector<buffer_element>& get_elements() const { return m_elements; }
	u32 get_stride() const { return m_stride; }

	std::vector<buffer_element>::iterator begin() { return m_elements.begin(); }
	std::vector<buffer_element>::iterator end() { return m_elements.end(); }
	std::vector<buffer_element>::const_iterator begin() const { return m_elements.begin(); }
	std::vector<buffer_element>::const_iterator end() const { return m_elements.end(); }

private:
	void calculate_offsets_and_stride()
	{
		size_t offset = 0;
		m_stride = 0;
		for (auto& element : m_elements)
		{
			element.Offset = offset;
			offset += element.Size;
			m_stride += element.Size;
		}
	}

	std::vector<buffer_element> m_elements;
	uint32_t m_stride = 0;
};

class vertex_buffer : public RefCounted
{
public:
    ~vertex_buffer() override = default;

	virtual void bind() const = 0;
	virtual void unbind() const = 0;

	virtual void set_data(const void* data, u32 size, u32 offset = 0) = 0;
	virtual void rt_set_data(const void* data, u32 size, u32 offset = 0) = 0;

	virtual void set_layout(const buffer_layout& layout) = 0;
	virtual const buffer_layout& get_layout() const = 0;


	static arc<vertex_buffer> create(u32 size);
	static arc<vertex_buffer> create(const void* data, u32 size);
};

class index_buffer : public RefCounted
{
public:
    ~index_buffer() override = default;

	virtual void bind() const = 0;
	virtual void unbind() const = 0;

	virtual void set_data(const void* buffer, u32 size, u32 offset = 0) = 0;

	virtual u32 get_count() const = 0;
	virtual u32 get_size() const = 0;

	static arc<index_buffer> create(u32 count);
	static arc<index_buffer> create(const void* data, u32 count);
};

} // end namespace kb::render::backend

#endif
