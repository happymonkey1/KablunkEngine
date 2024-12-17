#include "kablunkpch.h"
#include "Kablunk/Renderer/backend/shader.h"

#include "Kablunk/Renderer/Renderer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_context.h"
#include "kablunk/renderer/backend/vulkan/vulkan_shader.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

arc<shader> shader::create(const std::string& file_path, bool p_force_compile)
{
	switch (const auto backend = Singleton<Renderer>::get().get_render_backend_type())
	{
	case render_backend_type_t::vulkan:
	{
        const auto context = Singleton<Renderer>::get().get_graphics_context().as<vk::vulkan_context>();
        return static_cast<arc<shader>>(arc<vk::vulkan_shader>::Create(
            context->get_device()->get_vk_device(),
            file_path,
            p_force_compile
        ));
	}
	case render_backend_type_t::none:
        return arc<shader>{};
	default:
        KB_CORE_ASSERT(
            false,
            "[Shader::Create]: Unhandled render backend {}!",
            static_cast<std::underlying_type_t<backend::render_backend_type_t>>(backend)
        );
	    return arc<shader>{};
	}
}

shader_uniform::shader_uniform(std::string name, shader_uniform_type_t type, uint32_t size, uint32_t offset)
	: m_name{ std::move(name) }, m_type{ type }, m_size{ size }, m_offset{ offset }
{
}

std::string shader_uniform::get_uniform_type_to_string(shader_uniform_type_t type)
{
	if (type == shader_uniform_type_t::Bool)
		return std::string{ "Boolean" };
	if (type == shader_uniform_type_t::Int)
		return std::string{ "Int" };
	if (type == shader_uniform_type_t::Float)
		return std::string{ "Float" };

	return std::string{ "None" };
}

} // end kb::render::backend
