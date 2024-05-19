#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanMaterial.h"

#include "Kablunk/Core/Application.h"
#include "Platform/Vulkan/VulkanImage.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Platform/Vulkan/vulkan_api.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace kb
{
VulkanMaterial::VulkanMaterial(
    const ref<Shader>& shader,
    const std::string& name /*= ""*/
)
	: m_shader{ shader.As<VulkanShader>() }, m_name{ name }
{
	Init();
	render::register_shader_dependency(shader, ref<Material>{ this });
}

VulkanMaterial::VulkanMaterial(ref<Material> material, const std::string& name /*= ""*/)
	: m_shader{ material->GetShader() }, m_name{ name }
{
	if (name.empty())
		m_name = material->GetName();

	render::register_shader_dependency(m_shader.As<Shader>(), ref<Material>{ this });

	auto vulkan_material = material.As<VulkanMaterial>();
	m_uniform_storage_buffer = owning_buffer::Copy(
        vulkan_material->m_uniform_storage_buffer.get(),
        vulkan_material->m_uniform_storage_buffer.size()
    );
}

auto VulkanMaterial::rt_prepare() noexcept -> void
{
    m_descriptor_set_manager.rt_invalidate_and_update();
}

void VulkanMaterial::Init()
{
	AllocateStorage();

	m_material_flags |= static_cast<uint32_t>(MaterialFlag::DepthTest);
	m_material_flags |= static_cast<uint32_t>(MaterialFlag::Blend);

    const render::descriptor_set_manager_specification spec{
        .m_shader = m_shader,
        .m_debug_name = !m_name.empty() ? m_name : fmt::format("{}::{}", m_shader->GetName(), "material"),
        .m_start_set = 0,
        .m_end_set = 0,
        .m_default_resources = true
    };

    m_descriptor_set_manager = render::vulkan_descriptor_set_manager{ spec };

    const auto& white_texture = Application::Get().get_renderer_2d()->get_white_texture();

    for (const auto& [name, decl] :
        m_descriptor_set_manager.get_input_declarations())
    {
        switch (decl.m_type)
        {
        case render::render_pass_input_type_t::image_sampler_1d: [[fallthrough]];
        case render::render_pass_input_type_t::image_sampler_2d:
        {
            for (u32 i = 0; i < decl.m_count; ++i)
                m_descriptor_set_manager.set_input(std::string_view{ name }, white_texture, i);
        }
        }
    }

    KB_CORE_ASSERT(m_descriptor_set_manager.validate(), "[VulkanMaterial]: descriptor set validation failed?");
    m_descriptor_set_manager.bake();
}

void VulkanMaterial::Invalidate()
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
            const auto& vulkan_render_backend = Singleton<render::Renderer>::get()
                .get_render_backend()
                .backend();
            m_material_descriptor_sets[i] = vulkan_render_backend->rt_allocate_descriptor_set(
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

        const auto& white_texture = Application::Get().get_renderer_2d()->get_white_texture();
        for (const auto& [binding, vk_write_descriptor] : write_descriptors)
        {
            m_material_write_descriptors[binding] = vk_write_descriptor;
            m_material_descriptor_images[binding] = std::vector<ref<render::render_resource>>(vk_write_descriptor.descriptorCount);

            if (vk_write_descriptor.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            {
                for (size_t i = 0; i < vk_write_descriptor.descriptorCount; ++i)
                {
                    m_material_descriptor_images[binding][i] = white_texture->GetImage();
                }
            }
        }
    }
    else
    {
        log::core::warn(
            log::logger_tag_t::material,
            "shader {} has not set 0?",
            m_shader->GetName()
        );
    }
}

void VulkanMaterial::Bind()
{

}

void VulkanMaterial::Set(const std::string& name, float value)
{
	Set<float>(name, value);
}

void VulkanMaterial::Set(const std::string& name, int value)
{
	Set<int>(name, value);
}

void VulkanMaterial::Set(const std::string& name, uint32_t value)
{
	Set<uint32_t>(name, value);
}

void VulkanMaterial::Set(const std::string& name, bool value)
{
	Set<bool>(name, value);
}

void VulkanMaterial::Set(const std::string& name, const glm::vec2& value)
{
	Set<vec2_packed>(name, value);
}

void VulkanMaterial::Set(const std::string& name, const glm::vec3& value)
{
	Set<vec3_packed>(name, value);
}

void VulkanMaterial::Set(const std::string& name, const glm::vec4& value)
{
	Set<glm::vec4>(name, value);
}

void VulkanMaterial::Set(const std::string& name, const glm::ivec2& value)
{
	Set<glm::ivec2>(name, value);
}

void VulkanMaterial::Set(const std::string& name, const glm::ivec3& value)
{
	Set<glm::ivec3>(name, value);
}

void VulkanMaterial::Set(const std::string& name, const glm::ivec4& value)
{
	Set<glm::ivec4>(name, value);
}

void VulkanMaterial::Set(const std::string& name, const glm::mat3& value)
{
	Set<glm::mat3>(name, value);
}

void VulkanMaterial::Set(const std::string& name, const glm::mat4& value)
{
	Set<glm::mat4>(name, value);
}

void VulkanMaterial::Set(const std::string& name, const ref<Texture2D>& texture)
{
	SetVulkanDescriptor(name, texture);
}

void VulkanMaterial::Set(const std::string& name, const ref<Texture2D>& texture, uint32_t array_index)
{
	SetVulkanDescriptor(name, texture, array_index);
}

void VulkanMaterial::Set(const std::string& name, const ref<Image2D>& image)
{
	SetVulkanDescriptor(name, image);
}


void VulkanMaterial::AllocateStorage()
{
	const auto& shader_buffers = m_shader->GetShaderBuffers();

	if (!shader_buffers.empty())
	{
		uint32_t size = 0;
		for (const auto& [name, shader_buffer] : shader_buffers)
			size += shader_buffer.size;

		m_uniform_storage_buffer.Allocate(size);
		m_uniform_storage_buffer.zero();
	}
}

void VulkanMaterial::OnShaderReloaded()
{
	// #TODO implement
}

void VulkanMaterial::SetVulkanDescriptor(const std::string& name, const ref<Texture2D>& texture)
{
    m_descriptor_set_manager.set_input(
        std::string_view{ name },
        texture
    );
}

void VulkanMaterial::SetVulkanDescriptor(const std::string& name, const ref<Texture2D>& texture, uint32_t array_index)
{
    m_descriptor_set_manager.set_input(
        std::string_view{ name },
        texture,
        array_index
    );
}

void VulkanMaterial::SetVulkanDescriptor(const std::string& name, const ref<Image2D>& image)
{
    m_descriptor_set_manager.set_input(
        std::string_view{ name },
        image
    );
}

const ShaderUniform* VulkanMaterial::FindUniformDeclaration(const std::string& name)
{
	const auto& shader_buffer = m_shader->GetShaderBuffers();

	KB_CORE_ASSERT(shader_buffer.size() <= 1, "only one shader buffer is currently supported!");

	if (!shader_buffer.empty())
	{
		const ShaderBuffer& buffer = shader_buffer.begin()->second;
		if (buffer.uniforms.find(name) != buffer.uniforms.end())
			return &buffer.uniforms.at(name);
		else
			return nullptr;
	}
	else
		return nullptr;
}

const ShaderResourceDeclaration* VulkanMaterial::FindResourceDeclaration(const std::string& name)
{
	const auto& resources = m_shader->GetResources();
	for (const auto& [nname, resource] : resources) // #TODO why can't we just access via indexing? 
		if (resource.GetName() == name)
			return &resource;

	return nullptr;
}

bool& VulkanMaterial::GetBool(const std::string& name)
{
	return Get<bool>(name);
}

float& VulkanMaterial::GetFloat(const std::string& name)
{
	return Get<float>(name);
}

int& VulkanMaterial::GetInt(const std::string& name)
{
	return Get<int>(name);
}

uint32_t& VulkanMaterial::GetUInt(const std::string& name)
{
	return Get<uint32_t>(name);
}

glm::vec2& VulkanMaterial::GetVec2(const std::string& name)
{
	return Get<glm::vec2>(name);
}

glm::vec3& VulkanMaterial::GetVec3(const std::string& name)
{
	return Get<glm::vec3>(name);
}

glm::vec4& VulkanMaterial::GetVec4(const std::string& name)
{
	return Get<glm::vec4>(name);
}

glm::mat3& VulkanMaterial::GetMat3(const std::string& name)
{
	return Get<glm::mat3>(name);
}

glm::mat4& VulkanMaterial::GetMat4(const std::string& name)
{
	return Get<glm::mat4>(name);
}

ref<Texture2D> VulkanMaterial::GetTexture2D(const std::string& name)
{
    KB_CORE_ASSERT(false, "disabled!");

	//return get_resource<Texture2D>(name);
    return ref<Texture2D>{};
}

ref<Texture2D> VulkanMaterial::TryGetTexture2D(const std::string& name)
{
    KB_CORE_ASSERT(false, "disabled!");

    //return get_resource<Texture2D>(name);
    return ref<Texture2D>{};
}

}
