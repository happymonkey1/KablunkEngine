#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_SHADER_H
#define KABLUNK_PLATFORM_VULKAN_SHADER_H

#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/ShaderUniform.h"

#include <vulkan/vulkan.h>

namespace kb
{
	class VulkanShader : public Shader
	{
	public:
		struct UniformBuffer
		{
			VkDescriptorBufferInfo descriptor;
			uint32_t size = 0;
			uint32_t binding_point = 0;
			std::string name;
			VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		};

		struct StorageBuffer
		{
			VkDescriptorBufferInfo descriptor;
			uint32_t size = 0;
			uint32_t binding_point = 0;
			std::string name;
			VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		};

		struct ImageSampler
		{
			uint32_t binding_point = 0;
			uint32_t descriptor_set = 0;
			uint32_t array_size = 0;
			std::string name;
			VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		};

		struct PushConstantRange
		{
			VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
			uint32_t offset = 0;
			uint32_t size = 0;
		};


		VulkanShader(const std::string& path, bool force_compile);
		virtual ~VulkanShader();

		virtual void destroy() override;

		virtual void Reload(bool force_compile = false) override;
		virtual size_t GetHash() const override;

		virtual void AddShaderReloadedCallback(const ShaderReloadedCallback& callback) override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;
		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) override;

		virtual const std::string& GetName() const { return m_name; };
		virtual const std::unordered_map<std::string, ShaderBuffer>& GetShaderBuffers() const override { return m_buffers; }
		virtual const std::unordered_map<std::string, ShaderResourceDeclaration>& GetResources() const override { return m_resources; };
		virtual RendererID GetRendererID() const { KB_CORE_ASSERT(false, "does not apply for Vulkan!"); return 0; };

		const std::vector<VkPipelineShaderStageCreateInfo>& GetPipelineShaderStageCreateInfos() const { return m_pipeline_shader_stage_create_infos; }

		VkDescriptorSet GetDescriptorSet() { return m_descriptor_set; }
		VkDescriptorSetLayout GetDescriptorSetLayout(uint32_t set) { return m_descriptor_set_layouts.at(set); }
		std::vector<VkDescriptorSetLayout> GetAllDescriptorSetLayouts();

		UniformBuffer& GetUniformBuffer(uint32_t binding = 0, uint32_t set = 0) 
		{ 
			KB_CORE_ASSERT(m_shader_descriptor_sets.at(set).uniform_buffers.size() > binding, "out of range!");
			return *m_shader_descriptor_sets.at(set).uniform_buffers.at(binding);
		}
		uint32_t GetUniformBufferCount(uint32_t set = 0)
		{
			if (m_shader_descriptor_sets.size() < set)
				return 0;

			return static_cast<uint32_t>(m_shader_descriptor_sets[set].uniform_buffers.size());
		}

		struct ShaderDescriptorSet
		{
			std::unordered_map<uint32_t, UniformBuffer*> uniform_buffers;
			std::unordered_map<uint32_t, StorageBuffer*> storage_buffers;
			std::unordered_map<uint32_t, ImageSampler> image_samplers;
			std::unordered_map<uint32_t, ImageSampler> storage_images;

			std::unordered_map<std::string, VkWriteDescriptorSet> write_descriptor_sets;

			operator bool() const { return !(uniform_buffers.empty() && storage_buffers.empty() && image_samplers.empty() && storage_images.empty()); }
		};

		const std::vector<ShaderDescriptorSet>& GetShaderDescriptorSets() const { return m_shader_descriptor_sets; }
		bool HasDescriptorSet(uint32_t set) const { return m_type_counts.find(set) != m_type_counts.end(); }

		const std::vector<PushConstantRange>& GetPushConstantRanges() const { return m_push_constant_ranges; }

		struct ShaderMaterialDescriptorSet
		{
			VkDescriptorPool pool = nullptr;
			std::vector<VkDescriptorSet> descriptor_sets;
		};

		ShaderMaterialDescriptorSet AllocateDescriptorSet(uint32_t set = 0);
		ShaderMaterialDescriptorSet CreateDescriptorSets(uint32_t set = 0);
		ShaderMaterialDescriptorSet CreateDescriptorSets(uint32_t set, uint32_t number_of_sets);
		const VkWriteDescriptorSet* GetDescriptorSet(const std::string& name, uint32_t set = 0) const;

		static void ClearUniformBuffers();
	private:
		std::unordered_map<VkShaderStageFlagBits, std::string> PreProcess(const std::string& source);
		void CompileOrGetVulkanBinaries(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& output_binary, bool force_compile);
		void LoadAndCreateShaders(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shader_data);
		void Reflect(VkShaderStageFlagBits shader_stage, const std::vector<uint32_t>& shader_data);
		void ReflectAllShaderStages(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shader_data);

		void CreateDescriptors();
	private:
		std::vector<VkPipelineShaderStageCreateInfo> m_pipeline_shader_stage_create_infos;
		std::unordered_map<VkShaderStageFlagBits, std::string> m_shader_source;
		std::string m_file_path;
		std::string m_name;
        size_t m_hash;

		std::vector<ShaderDescriptorSet> m_shader_descriptor_sets;
		
		std::vector<PushConstantRange> m_push_constant_ranges;
		std::unordered_map<std::string, ShaderResourceDeclaration> m_resources;

		std::unordered_map<std::string, ShaderBuffer> m_buffers;

		std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;
		VkDescriptorSet m_descriptor_set;

		std::unordered_map<uint32_t, std::vector<VkDescriptorPoolSize>> m_type_counts;

		// flag for whether this shader has been freed
		bool m_has_destroyed = false;
	};
}

#endif
