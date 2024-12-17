#include "kablunkpch.h"

#include "kablunk/renderer/backend/vulkan/vulkan_material.h"

#include "Kablunk/Core/Application.h"

#include "Kablunk/renderer/renderer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_image.h"
#include "kablunk/renderer/backend/vulkan/vulkan_shader.h"
#include "kablunk/renderer/backend/vulkan/vulkan_texture.h"
#include "kablunk/renderer/backend/vulkan/vulkan_api.h"
#include "kablunk/renderer/backend/vulkan/VulkanRendererAPI.h"
#include "Kablunk/renderer/backend/vulkan/vulkan_render_backend.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

// TODO: can we re-use descriptor set for same shader?

vulkan_material::vulkan_material(
    const arc<shader>& shader,
    const std::string& name /*= ""*/
)
	: m_shader{ shader.As<vulkan_shader>() }, m_name{ !name.empty() ? name : shader->get_name() }
{
	Init();
	render::register_shader_dependency(shader, arc<material>{ this });
}

vulkan_material::vulkan_material(arc<material> p_material, const std::string& name /*= ""*/)
	: m_shader{ p_material->get_shader() }, m_name{ name }
{
	if (name.empty())
		m_name = p_material->get_name();

    Init();
	render::register_shader_dependency(m_shader.As<shader>(), arc<material>{ this });

	auto vk_material = p_material.As<vulkan_material>();
	m_uniform_storage_buffer = owning_buffer::copy(
        vk_material->m_uniform_storage_buffer.get(),
        vk_material->m_uniform_storage_buffer.size()
    );
}

auto vulkan_material::rt_prepare() noexcept -> void
{
    m_descriptor_set_manager.rt_invalidate_and_update();
}

void vulkan_material::Init()
{
	AllocateStorage();

	m_material_flags |= static_cast<u32>(MaterialFlag::DepthTest);
	m_material_flags |= static_cast<u32>(MaterialFlag::Blend);

    const descriptor_set_manager_specification spec{
        .m_shader = m_shader,
        .m_debug_name = !m_name.empty() ? m_name : fmt::format("{}::{}", m_shader->get_name(), "material"),
        .m_start_set = 0,
        .m_end_set = 0,
        .m_default_resources = true
    };

    m_descriptor_set_manager = vulkan_descriptor_set_manager{ spec };

    const auto& white_texture = Singleton<Renderer>::get().get_white_texture();

    for (const auto& [name, decl] :
        m_descriptor_set_manager.get_input_declarations())
    {
        switch (decl.m_type)
        {
        case render_pass_input_type_t::image_sampler_1d: [[fallthrough]];
        case render_pass_input_type_t::image_sampler_2d:
        {
            for (u32 i = 0; i < decl.m_count; ++i)
                m_descriptor_set_manager.set_input(std::string_view{ name }, white_texture, i);
        }
        }
    }

    KB_CORE_ASSERT(m_descriptor_set_manager.validate(), "[VulkanMaterial]: descriptor set validation failed?");
    m_descriptor_set_manager.bake();
}

void vulkan_material::invalidate()
{
    // descriptor set 0 is allocated based on shader layout
    if (m_shader->HasDescriptorSet(0))
    {
        const VkDescriptorSetLayout vk_descriptor_set_layout = m_shader->get_vk_descriptor_set_layout(0);
        VkDescriptorSetAllocateInfo descriptor_set_allocate_info = vk::get_descriptor_set_alloc_info(
            &vk_descriptor_set_layout
        );

        const auto frames_in_flight = render::get_frames_in_flight();
        for (u32 i = 0; i < frames_in_flight; ++i)
        {
            m_material_descriptor_sets[i] = vulkan_render_backend::rt_allocate_descriptor_set(
                descriptor_set_allocate_info
            );
        }

        // sort into map which is sorted by binding
        const auto& shader_descriptor_sets = m_shader->GetShaderDescriptorSets();
        std::map<u32, VkWriteDescriptorSet> write_descriptors{};

        for (const auto& [name, vk_write_descriptor] : shader_descriptor_sets[0].write_descriptor_sets)
        {
            if (
                vk_write_descriptor.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                vk_write_descriptor.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
                vk_write_descriptor.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                )
            {
                write_descriptors[vk_write_descriptor.dstBinding] = vk_write_descriptor;
            }
        }

        const auto& white_texture = Singleton<Renderer>::get().get_white_texture();
        for (const auto& [binding, vk_write_descriptor] : write_descriptors)
        {
            m_material_write_descriptors[binding] = vk_write_descriptor;
            m_material_descriptor_images[binding] = std::vector<arc<render::render_resource>>(vk_write_descriptor.descriptorCount);

            if (vk_write_descriptor.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            {
                for (size_t i = 0; i < vk_write_descriptor.descriptorCount; ++i)
                {
                    m_material_descriptor_images[binding][i] = arc<render::render_resource>{
                        white_texture->get_image()
                    };
                }
            }
        }
    }
    else
    {
        log::core::warn(
            log::logger_tag_t::material,
            "shader {} has not set 0?",
            m_shader->get_name()
        );
    }
}

void vulkan_material::bind()
{

}

void vulkan_material::set(const std::string& name, float value)
{
	Set<float>(name, value);
}

void vulkan_material::set(const std::string& name, int value)
{
	Set<int>(name, value);
}

void vulkan_material::set(const std::string& name, u32 value)
{
	Set<u32>(name, value);
}

void vulkan_material::set(const std::string& name, bool value)
{
	Set<bool>(name, value);
}

void vulkan_material::set(const std::string& name, const glm::vec2& value)
{
	Set<vec2_packed>(name, value);
}

void vulkan_material::set(const std::string& name, const glm::vec3& value)
{
	Set<vec3_packed>(name, value);
}

void vulkan_material::set(const std::string& name, const glm::vec4& value)
{
	Set<glm::vec4>(name, value);
}

void vulkan_material::set(const std::string& name, const glm::ivec2& value)
{
	Set<glm::ivec2>(name, value);
}

void vulkan_material::set(const std::string& name, const glm::ivec3& value)
{
	Set<glm::ivec3>(name, value);
}

void vulkan_material::set(const std::string& name, const glm::ivec4& value)
{
	Set<glm::ivec4>(name, value);
}

void vulkan_material::set(const std::string& name, const glm::mat3& value)
{
	Set<glm::mat3>(name, value);
}

void vulkan_material::set(const std::string& name, const glm::mat4& value)
{
	Set<glm::mat4>(name, value);
}

void vulkan_material::set(const std::string& name, const arc<texture_2d>& texture)
{
	SetVulkanDescriptor(name, texture);
}

void vulkan_material::set(const std::string& name, const arc<texture_2d>& texture, u32 array_index)
{
	SetVulkanDescriptor(name, texture, array_index);
}

void vulkan_material::set(const std::string& name, const arc<image_2d>& image)
{
	SetVulkanDescriptor(name, image);
}


void vulkan_material::AllocateStorage()
{
	const auto& shader_buffers = m_shader->get_shader_buffers();

	if (!shader_buffers.empty())
	{
		u32 size = 0;
		for (const auto& [name, shader_buffer] : shader_buffers)
			size += shader_buffer.size;

		m_uniform_storage_buffer.allocate(size);
		m_uniform_storage_buffer.zero();
	}
}

void vulkan_material::OnShaderReloaded()
{
	// #TODO implement
}

void vulkan_material::SetVulkanDescriptor(const std::string& name, const arc<texture_2d>& texture)
{
    m_descriptor_set_manager.set_input(
        std::string_view{ name },
        texture
    );
}

void vulkan_material::SetVulkanDescriptor(const std::string& name, const arc<texture_2d>& texture, u32 array_index)
{
    m_descriptor_set_manager.set_input(
        std::string_view{ name },
        texture,
        array_index
    );
}

void vulkan_material::SetVulkanDescriptor(const std::string& name, const arc<image_2d>& image)
{
    m_descriptor_set_manager.set_input(
        std::string_view{ name },
        image
    );
}

void vulkan_material::SetVulkanDescriptor(const std::string& p_name, const arc<image_view>& p_image)
{
    m_descriptor_set_manager.set_input(
        std::string_view{ p_name },
        p_image
    );
}

const shader_uniform* vulkan_material::FindUniformDeclaration(const std::string& name)
{
	const auto& shader_buffers = m_shader->get_shader_buffers();

	KB_CORE_ASSERT(shader_buffers.size() <= 1, "only one shader buffer is currently supported!");

	if (!shader_buffers.empty())
	{
		const shader_buffer_t& buffer = shader_buffers.begin()->second;
		if (buffer.uniforms.find(name) != buffer.uniforms.end())
			return &buffer.uniforms.at(name);

	    return nullptr;
	}

    return nullptr;
}

const shader_resource_declaration* vulkan_material::FindResourceDeclaration(const std::string& name)
{
	const auto& resources = m_shader->get_resources();
	for (const auto& [nname, resource] : resources) // #TODO why can't we just access via indexing?
		if (resource.get_name() == name)
			return &resource;

	return nullptr;
}

bool& vulkan_material::get_bool(const std::string& name)
{
	return Get<bool>(name);
}

float& vulkan_material::get_float(const std::string& name)
{
	return Get<float>(name);
}

int& vulkan_material::get_int(const std::string& name)
{
	return Get<int>(name);
}

u32& vulkan_material::get_uint(const std::string& name)
{
	return Get<u32>(name);
}

glm::vec2& vulkan_material::get_vec2(const std::string& name)
{
	return Get<glm::vec2>(name);
}

glm::vec3& vulkan_material::get_vec3(const std::string& name)
{
	return Get<glm::vec3>(name);
}

glm::vec4& vulkan_material::get_vec4(const std::string& name)
{
	return Get<glm::vec4>(name);
}

glm::mat3& vulkan_material::get_mat3(const std::string& name)
{
	return Get<glm::mat3>(name);
}

glm::mat4& vulkan_material::get_mat4(const std::string& name)
{
	return Get<glm::mat4>(name);
}

arc<texture_2d> vulkan_material::get_texture_2d(const std::string& name)
{
    KB_CORE_ASSERT(false, "disabled!");

	//return get_resource<Texture2D>(name);
    return arc<texture_2d>{};
}

arc<texture_2d> vulkan_material::try_get_texture_2d(const std::string& name)
{
    KB_CORE_ASSERT(false, "disabled!");

    //return get_resource<Texture2D>(name);
    return arc<texture_2d>{};
}

} // end namespace kb::render::backend::vk
