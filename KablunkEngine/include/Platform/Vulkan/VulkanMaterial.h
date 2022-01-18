#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_MATERIAL_H
#define KABLUNK_PLATFORM_VULKAN_MATERIAL_H

#include "Kablunk/Renderer/Material.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/Image.h"


#include "Platform/Vulkan/VulkanShader.h"

#include <vulkan/vulkan.h>

namespace Kablunk
{

	class VulkanMaterial : public Material
	{
	public:
		VulkanMaterial(const IntrusiveRef<Shader>& shader, const std::string& name = "");
		VulkanMaterial(IntrusiveRef<Material> material, const std::string& name = "");
		virtual ~VulkanMaterial() override;

		virtual void Invalidate() override;
		virtual void Bind() override;

		virtual void Set(const std::string & name, float value) override;
		virtual void Set(const std::string & name, int value) override;
		virtual void Set(const std::string & name, uint32_t value) override;
		virtual void Set(const std::string & name, bool value) override;
		virtual void Set(const std::string & name, const glm::vec2 & value) override;
		virtual void Set(const std::string & name, const glm::vec3 & value) override;
		virtual void Set(const std::string & name, const glm::vec4 & value) override;
		virtual void Set(const std::string & name, const glm::ivec2 & value) override;
		virtual void Set(const std::string& name, const glm::ivec3& value) override;
		virtual void Set(const std::string& name, const glm::ivec4& value) override;
		virtual void Set(const std::string & name, const glm::mat3 & value) override;
		virtual void Set(const std::string & name, const glm::mat4 & value) override;
		virtual void Set(const std::string & name, const IntrusiveRef<Texture2D>& texture) override;
		virtual void Set(const std::string & name, const IntrusiveRef<Texture2D>& texture, uint32_t array_index) override;
		virtual void Set(const std::string & name, const IntrusiveRef<Image2D>& image) override;

		virtual float& GetFloat(const std::string & name) override;
		virtual int& GetInt(const std::string & name) override;
		virtual uint32_t& GetUInt(const std::string & name) override;
		virtual glm::vec2& GetVec2(const std::string & name) override;
		virtual glm::vec3& GetVec3(const std::string & name) override;
		virtual glm::vec4& GetVec4(const std::string & name) override;
		virtual glm::mat3& GetMat3(const std::string & name) override;
		virtual glm::mat4& GetMat4(const std::string & name) override;
		virtual IntrusiveRef<Texture2D> GetTexture2D(const std::string & name) override;
		virtual IntrusiveRef<Texture2D> TryGetTexture2D(const std::string & name) override;

		template <typename T>
		void Set(const std::string& name, const T& value)
		{
			auto decl = FindUniformDeclaration(name);
			KB_CORE_ASSERT(decl, "Could not find uniform!");
			if (!decl)
				return;

			auto& buffer = m_uniform_storage_buffer;
			buffer.Write((uint8_t*)&value, decl->GetSize(), decl->GetOffset());
		}

		template<typename T>
		T& Get(const std::string& name)
		{
			auto decl = FindUniformDeclaration(name);
			KB_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			auto& buffer = m_uniform_storage_buffer;
			return buffer.Read<T>(decl->GetOffset());
		}

		template<typename T>
		IntrusiveRef<T> GetResource(const std::string& name)
		{
			auto decl = FindResourceDeclaration(name);
			KB_CORE_ASSERT(decl, "Could not find uniform with name 'x'");
			uint32_t slot = decl->GetRegister();
			KB_CORE_ASSERT(slot < m_textures.size(), "Texture slot is invalid!");
			return m_textures[slot];
		}

		template<typename T>
		IntrusiveRef<T> TryGetResource(const std::string& name)
		{
			auto decl = FindResourceDeclaration(name);
			if (!decl)
				return nullptr;

			uint32_t slot = decl->GetRegister();
			if (slot >= m_textures.size())
				return nullptr;

			return m_textures[slot];
		}

		virtual uint32_t GetFlags() const override { return m_material_flags; }
		virtual bool GetFlag(MaterialFlag flag) const override { return (uint32_t)flag & m_material_flags; }
		virtual void SetFlag(MaterialFlag flag, bool value = true) override
		{
			if (value)
				m_material_flags |= (uint32_t)flag;
			else
				m_material_flags &= ~(uint32_t)flag;
		}

		virtual IntrusiveRef<Shader> GetShader() override { return m_shader; }
		virtual const std::string& GetName() const override { return m_name; }

		Buffer GetUniformStorageBuffer() { return m_uniform_storage_buffer; }
		const Buffer& GetUniformStorageBuffer() const { return m_uniform_storage_buffer; }

		void RT_UpdateForRendering(const std::vector<std::vector<VkWriteDescriptorSet>>& uniformBufferWriteDescriptors = std::vector<std::vector<VkWriteDescriptorSet>>());
		void InvalidateDescriptorSets();

		VkDescriptorSet GetDescriptorSet(uint32_t index) const { return !m_descriptor_sets[index].descriptor_sets.empty() ? m_descriptor_sets[index].descriptor_sets[0] : nullptr; }
	private:
		void Init();
		void AllocateStorage();
		void OnShaderReloaded();

		void SetVulkanDescriptor(const std::string& name, const IntrusiveRef<Texture2D>& texture);
		void SetVulkanDescriptor(const std::string& name, const IntrusiveRef<Texture2D>& texture, uint32_t array_index);
		void SetVulkanDescriptor(const std::string& name, const IntrusiveRef<Image2D>& images);

		const ShaderUniform* FindUniformDeclaration(const std::string& name);
		const ShaderResourceDeclaration* FindResourceDeclaration(const std::string& name);
	private:
		IntrusiveRef<Shader> m_shader;

		std::vector<IntrusiveRef<Image>> m_textures;
		std::vector<std::vector<IntrusiveRef<Texture2D>>> m_texture_array;
		std::vector<IntrusiveRef<Image>> m_images;

		enum class PendingDescriptorType
		{
			None = 0, Texture2D, Image2D
		};

		struct PendingDescriptor
		{
			PendingDescriptorType type = PendingDescriptorType::None;
			VkWriteDescriptorSet write_descriptor_set;
			VkDescriptorImageInfo image_info;
			IntrusiveRef<Texture2D> texture;
			IntrusiveRef<Image2D> image;
			VkDescriptorImageInfo submitted_image_info{};
		};

		struct PendingDescriptorArray
		{
			PendingDescriptorType type = PendingDescriptorType::None;
			VkWriteDescriptorSet write_descriptor_set;
			std::vector<VkDescriptorImageInfo> image_infos;
			std::vector<IntrusiveRef<Texture2D>> textures;
			std::vector<IntrusiveRef<Image2D>> images;
			VkDescriptorImageInfo submitted_image_info{};
		};
		std::unordered_map<uint32_t, std::shared_ptr<PendingDescriptor>> m_resident_descriptors;
		std::unordered_map<uint32_t, std::shared_ptr<PendingDescriptorArray>> m_resident_descriptor_array;
		std::vector<std::shared_ptr<PendingDescriptor>> m_pending_descriptors; // #TODO weak ref

		uint32_t m_material_flags;

		std::unordered_map<uint32_t, uint64_t> m_image_hashes;

		std::vector<std::vector<VkWriteDescriptorSet>> m_write_descriptors;
		std::vector<bool> m_dirty_descriptor_sets;

		std::unordered_map<std::string, VkDescriptorImageInfo> m_image_infos;

		Buffer m_uniform_storage_buffer;

		VulkanShader::ShaderMaterialDescriptorSet m_descriptor_sets[3];

		std::string m_name;
	};

}

#endif
