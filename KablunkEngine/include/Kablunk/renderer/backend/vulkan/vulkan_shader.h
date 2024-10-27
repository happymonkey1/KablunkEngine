#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_SHADER_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_SHADER_H

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/renderer/backend/shader.h"
#include "Kablunk/renderer/backend/shader_resource_declaration.h"

#include <vulkan/vulkan.h>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

class vulkan_shader final : public shader
{
public:
	struct vk_uniform_buffer_t
	{
		VkDescriptorBufferInfo descriptor;
		u32 size = 0;
		u32 binding_point = 0;
		std::string name;
		VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	};

	struct vk_storage_buffer_t
	{
		VkDescriptorBufferInfo descriptor;
		u32 size = 0;
		u32 binding_point = 0;
		std::string name;
		VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	};

	struct vk_image_sampler_t
	{
        u32 binding_point = 0;
        u32 descriptor_set = 0;
        u32 array_size = 0;
        u32 m_dimension = 0;
		std::string name;
		VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	};

	struct vk_push_constant_range_t
	{
		VkShaderStageFlagBits shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        u32 offset = 0;
        u32 size = 0;
	};


	vulkan_shader(const std::string& path, bool force_compile);
    ~vulkan_shader() override = default;

	void destroy() override;

	void reload(bool force_compile = false) override;
	size_t get_hash() const override;

	void add_shader_reloaded_callback(const ShaderReloadedCallback& callback) override;

	void bind() const override;
	void unbind() const override;

	void set_mat4(const std::string& name, const glm::mat4& value) override;
	void set_float(const std::string& name, float value) override;
	void set_float2(const std::string& name, const glm::vec2& value) override;
	void set_float3(const std::string& name, const glm::vec3& value) override;
	void set_float4(const std::string& name, const glm::vec4& value) override;
	void set_int(const std::string& name, int value) override;
	void set_int_array(const std::string& name, int* values, uint32_t count) override;

	const std::string& get_name() const override { return m_name; }

	const kb::unordered_flat_map<std::string, shader_buffer_t>& get_shader_buffers() const override
	{
	    return m_buffers;
	}

	const kb::unordered_flat_map<std::string, shader_resource_declaration>& get_resources() const override
	{
	    return m_resources;
	}

	RendererID get_renderer_id() const override
	{
	    KB_CORE_ASSERT(false, "does not apply for Vulkan!");
	    return 0;
	}

	const std::vector<VkPipelineShaderStageCreateInfo>& GetPipelineShaderStageCreateInfos() const { return m_pipeline_shader_stage_create_infos; }

	VkDescriptorSetLayout get_vk_descriptor_set_layout(u32 set) const { return m_descriptor_set_layouts.at(set); }
	std::vector<VkDescriptorSetLayout> GetAllDescriptorSetLayouts();

	vk_uniform_buffer_t& GetUniformBuffer(uint32_t binding = 0, uint32_t set = 0)
	{
		KB_CORE_ASSERT(m_shader_descriptor_sets.at(set).uniform_buffers.size() > binding, "out of range!");
		return *m_shader_descriptor_sets.at(set).uniform_buffers.at(binding);
	}

	uint32_t GetUniformBufferCount(uint32_t set = 0) const
    {
		if (m_shader_descriptor_sets.size() < set)
			return 0;

		return static_cast<uint32_t>(m_shader_descriptor_sets[set].uniform_buffers.size());
	}

	struct ShaderDescriptorSet
	{
		kb::unordered_flat_map<uint32_t, vk_uniform_buffer_t*> uniform_buffers;
		kb::unordered_flat_map<uint32_t, vk_storage_buffer_t*> storage_buffers;
		kb::unordered_flat_map<uint32_t, vk_image_sampler_t> image_samplers;
		kb::unordered_flat_map<uint32_t, vk_image_sampler_t> storage_images;

        kb::unordered_flat_map<std::string, VkWriteDescriptorSet> write_descriptor_sets;

		operator bool() const { return !(uniform_buffers.empty() && storage_buffers.empty() && image_samplers.empty() && storage_images.empty()); }
	};

	const std::vector<ShaderDescriptorSet>& GetShaderDescriptorSets() const { return m_shader_descriptor_sets; }
	bool HasDescriptorSet(uint32_t set) const { return m_type_counts.find(set) != m_type_counts.end(); }

	const std::vector<vk_push_constant_range_t>& GetPushConstantRanges() const { return m_push_constant_ranges; }

	struct ShaderMaterialDescriptorSet
	{
		VkDescriptorPool pool = nullptr;
		std::vector<VkDescriptorSet> descriptor_sets;
	};

	ShaderMaterialDescriptorSet allocate_descriptor_set(uint32_t set = 0) const;
	ShaderMaterialDescriptorSet CreateDescriptorSets(uint32_t set = 0);
	ShaderMaterialDescriptorSet CreateDescriptorSets(uint32_t set, uint32_t number_of_sets);
	const VkWriteDescriptorSet* GetDescriptorSet(const std::string& name, uint32_t set = 0) const;

	static void ClearUniformBuffers();
private:
	kb::unordered_flat_map<VkShaderStageFlagBits, std::string> PreProcess(const std::string& source);
	void CompileOrGetVulkanBinaries(kb::unordered_flat_map<VkShaderStageFlagBits, std::vector<uint32_t>>& output_binary, bool force_compile);
	void LoadAndCreateShaders(const kb::unordered_flat_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shader_data);
	void Reflect(VkShaderStageFlagBits shader_stage, const std::vector<uint32_t>& shader_data);
	void ReflectAllShaderStages(const kb::unordered_flat_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shader_data);

	void CreateDescriptors();
private:
	std::vector<VkPipelineShaderStageCreateInfo> m_pipeline_shader_stage_create_infos;
    kb::unordered_flat_map<VkShaderStageFlagBits, std::string> m_shader_source;
	std::string m_file_path;
	std::string m_name;
    size_t m_hash;

	std::vector<ShaderDescriptorSet> m_shader_descriptor_sets;
	std::vector<vk_push_constant_range_t> m_push_constant_ranges;
    kb::unordered_flat_map<std::string, shader_resource_declaration> m_resources;

    kb::unordered_flat_map<std::string, shader_buffer_t> m_buffers;

	std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;

    kb::unordered_flat_map<uint32_t, std::vector<VkDescriptorPoolSize>> m_type_counts;

	// flag for whether this shader has been freed
	bool m_has_destroyed = false;
};

} // end namespace kb

#endif
