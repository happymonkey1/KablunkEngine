#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_MATERIAL_H
#define KABLUNK_PLATFORM_VULKAN_MATERIAL_H

#include "Kablunk/Renderer/Material.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/Image.h"


#include "Platform/Vulkan/VulkanShader.h"

#include <vulkan/vulkan.h>

#include "Platform/Vulkan/vulkan_descriptor_set_manager.h"

namespace kb
{ // start namespace kb

class VulkanMaterial final : public Material
{
public:
	VulkanMaterial(const ref<Shader>& shader, const std::string& name = "");
	VulkanMaterial(ref<Material> material, const std::string& name = "");
    ~VulkanMaterial() override = default;

	void Invalidate() override;
	void Bind() override;

    // #TODO template this
	void Set(const std::string & name, float value) override;
	void Set(const std::string & name, int value) override;
	void Set(const std::string & name, uint32_t value) override;
	void Set(const std::string & name, bool value) override;
	void Set(const std::string & name, const glm::vec2 & value) override;
	void Set(const std::string & name, const glm::vec3 & value) override;
	void Set(const std::string & name, const glm::vec4 & value) override;
	void Set(const std::string & name, const glm::ivec2 & value) override;
	void Set(const std::string& name, const glm::ivec3& value) override;
	void Set(const std::string& name, const glm::ivec4& value) override;
	void Set(const std::string & name, const glm::mat3 & value) override;
	void Set(const std::string & name, const glm::mat4 & value) override;
	void Set(const std::string & name, const ref<Texture2D>& texture) override;
	void Set(const std::string & name, const ref<Texture2D>& texture, uint32_t array_index) override;
	void Set(const std::string & name, const ref<Image2D>& image) override;

    // #TODO template this
	bool& GetBool(const std::string& name) override;
	float& GetFloat(const std::string & name) override;
	int& GetInt(const std::string & name) override;
	uint32_t& GetUInt(const std::string & name) override;
	glm::vec2& GetVec2(const std::string & name) override;
	glm::vec3& GetVec3(const std::string & name) override;
	glm::vec4& GetVec4(const std::string & name) override;
	glm::mat3& GetMat3(const std::string & name) override;
	glm::mat4& GetMat4(const std::string & name) override;
	ref<Texture2D> GetTexture2D(const std::string & name) override;
	ref<Texture2D> TryGetTexture2D(const std::string & name) override;

	template <typename T>
	void Set(const std::string& name, const T& value)
	{
		auto decl = FindUniformDeclaration(name);
		KB_CORE_ASSERT(decl, "Could not find uniform!");
		if (!decl)
			return;

		m_uniform_storage_buffer.Write(reinterpret_cast<const uint8_t*>(&value), decl->GetSize(), decl->GetOffset());
	}

	template<typename T>
	T& Get(const std::string& name)
	{
		auto decl = FindUniformDeclaration(name);
		KB_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
		return m_uniform_storage_buffer.Read<T>(decl->GetOffset());
	}

	template<typename T>
	ref<T> get_resource(const std::string& name)
	{
        return m_descriptor_set_manager.get_input<T>(name);
	}

	template<typename T>
	ref<T> try_get_resource(const std::string& name)
	{
        return m_descriptor_set_manager.get_input<T>(name);
	}

	uint32_t GetFlags() const override { return m_material_flags; }
	bool GetFlag(MaterialFlag flag) const override { return (uint32_t)flag & m_material_flags; }
	void SetFlag(MaterialFlag flag, bool value = true) override
	{
		if (value)
			m_material_flags |= (uint32_t)flag;
		else
			m_material_flags &= ~(uint32_t)flag;
	}

	ref<Shader> GetShader() override { return m_shader.As<Shader>(); }
	const std::string& GetName() const override { return m_name; }

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

	void SetVulkanDescriptor(const std::string& name, const ref<Texture2D>& texture);
	void SetVulkanDescriptor(const std::string& name, const ref<Texture2D>& texture, uint32_t array_index);
	void SetVulkanDescriptor(const std::string& name, const ref<Image2D>& images);
    void SetVulkanDescriptor(const std::string& p_name, const ref<image_view>& p_image);

	const ShaderUniform* FindUniformDeclaration(const std::string& name);
	const ShaderResourceDeclaration* FindResourceDeclaration(const std::string& name);
private:
    ref<VulkanShader> m_shader{};
    std::string m_name{};

    render::vulkan_descriptor_set_manager m_descriptor_set_manager{};
    std::vector<VkDescriptorSet> m_material_descriptor_sets{};

    std::map<u32, std::vector<ref<render::render_resource>>> m_material_descriptor_images{};
    std::map<u32, VkWriteDescriptorSet> m_material_write_descriptors{};

    u32 m_material_flags = 0;
    owning_buffer m_uniform_storage_buffer{};
};

} // end namespace kb

#endif
