#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_MATERIAL_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_MATERIAL_H

#include "Kablunk/Renderer/backend/material.h"
#include "Kablunk/Renderer/backend/texture.h"
#include "Kablunk/Renderer/backend/image.h"


#include "kablunk/renderer/backend/vulkan/vulkan_shader.h"

#include <vulkan/vulkan.h>

#include "kablunk/renderer/backend/vulkan/vulkan_descriptor_set_manager.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

class vulkan_material final : public material
{
public:
	vulkan_material(const arc<shader>& shader, const std::string& name = "");
	vulkan_material(arc<material> p_material, const std::string& name = "");
    ~vulkan_material() override = default;

	void invalidate() override;
	void bind() override;

    // #TODO template this
	void set(const std::string & name, float value) override;
	void set(const std::string & name, int value) override;
	void set(const std::string & name, uint32_t value) override;
	void set(const std::string & name, bool value) override;
	void set(const std::string & name, const glm::vec2 & value) override;
	void set(const std::string & name, const glm::vec3 & value) override;
	void set(const std::string & name, const glm::vec4 & value) override;
	void set(const std::string & name, const glm::ivec2 & value) override;
	void set(const std::string& name, const glm::ivec3& value) override;
	void set(const std::string& name, const glm::ivec4& value) override;
	void set(const std::string & name, const glm::mat3 & value) override;
	void set(const std::string & name, const glm::mat4 & value) override;
	void set(const std::string & name, const arc<texture_2d>& texture) override;
	void set(const std::string & name, const arc<texture_2d>& texture, uint32_t array_index) override;
	void set(const std::string & name, const arc<image_2d>& image) override;

    // #TODO template this
	bool& get_bool(const std::string& name) override;
	float& get_float(const std::string & name) override;
	int& get_int(const std::string & name) override;
	uint32_t& get_uint(const std::string & name) override;
	glm::vec2& get_vec2(const std::string & name) override;
	glm::vec3& get_vec3(const std::string & name) override;
	glm::vec4& get_vec4(const std::string & name) override;
	glm::mat3& get_mat3(const std::string & name) override;
	glm::mat4& get_mat4(const std::string & name) override;
	arc<texture_2d> get_texture_2d(const std::string & name) override;
	arc<texture_2d> try_get_texture_2d(const std::string & name) override;

	template <typename T>
	void Set(const std::string& name, const T& value)
	{
		auto decl = FindUniformDeclaration(name);
		KB_CORE_ASSERT(decl, "Could not find uniform!");
		if (!decl)
			return;

		m_uniform_storage_buffer.write(reinterpret_cast<const uint8_t*>(&value), decl->get_size(), decl->get_offset());
	}

	template<typename T>
	T& Get(const std::string& name)
	{
		auto decl = FindUniformDeclaration(name);
		KB_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
		return m_uniform_storage_buffer.read<T>(decl->get_offset());
	}

	template<typename T>
	arc<T> get_resource(const std::string& name)
	{
        return m_descriptor_set_manager.get_input<T>(name);
	}

	template<typename T>
	arc<T> try_get_resource(const std::string& name)
	{
        return m_descriptor_set_manager.get_input<T>(name);
	}

	uint32_t get_flags() const override { return m_material_flags; }
	bool get_flag(MaterialFlag flag) const override { return (uint32_t)flag & m_material_flags; }
	void set_flag(MaterialFlag flag, bool value = true) override
	{
		if (value)
			m_material_flags |= (uint32_t)flag;
		else
			m_material_flags &= ~(uint32_t)flag;
	}

	arc<shader> get_shader() override { return m_shader.As<shader>(); }
	const std::string& get_name() const override { return m_name; }

	owning_buffer& get_uniform_storage_buffer() { return m_uniform_storage_buffer; }
	const owning_buffer& get_uniform_storage_buffer() const { return m_uniform_storage_buffer; }

    // retrieves the descriptor set for the frame, invalidating and updating descriptor set
    // if there are changes
	auto get_vk_descriptor_set(uint32_t frame_index) noexcept -> VkDescriptorSet
	{
        const auto index_opt = m_descriptor_set_manager.get_first_set_index();
        if (!index_opt)
        {
            // #TODO should look into this...
#if 0
            log::core::warn(
                log::logger_tag_t::material,
                "[vulkan_material]: Could not find a set descriptor set index in material '{}'!",
                m_name
            );
#endif
            return nullptr;
        }

        rt_prepare();
        KB_CORE_ASSERT(
            m_descriptor_set_manager.get_descriptor_sets(frame_index).size() == 1,
            "assumption invalidated!"
        );
        return m_descriptor_set_manager.get_descriptor_sets(frame_index)[0];
	}

    auto rt_prepare() noexcept -> void;

private:
	void Init();
	void AllocateStorage();
	void OnShaderReloaded();

	void SetVulkanDescriptor(const std::string& name, const arc<texture_2d>& texture);
	void SetVulkanDescriptor(const std::string& name, const arc<texture_2d>& texture, uint32_t array_index);
	void SetVulkanDescriptor(const std::string& name, const arc<image_2d>& images);
    void SetVulkanDescriptor(const std::string& p_name, const arc<image_view>& p_image);

	const shader_uniform* FindUniformDeclaration(const std::string& name);
	const shader_resource_declaration* FindResourceDeclaration(const std::string& name);
private:
    arc<vulkan_shader> m_shader{};
    std::string m_name{};

    vulkan_descriptor_set_manager m_descriptor_set_manager{};
    std::vector<VkDescriptorSet> m_material_descriptor_sets{};

    std::map<u32, std::vector<arc<render::render_resource>>> m_material_descriptor_images{};
    std::map<u32, VkWriteDescriptorSet> m_material_write_descriptors{};

    u32 m_material_flags = 0;
    owning_buffer m_uniform_storage_buffer{};
};

} // end namespace kb::render::backend::vk

#endif
