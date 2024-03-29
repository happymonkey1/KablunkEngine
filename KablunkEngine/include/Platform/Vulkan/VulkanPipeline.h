#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_PIPELINE_H
#define KABLUNK_PLATFORM_VULKAN_PIPELINE_H

#include "Kablunk/Renderer/Pipeline.h"

#include "Platform/Vulkan/VulkanShader.h"

#include <vulkan/vulkan.h>

namespace kb
{ // start namespace kb
class VulkanPipeline final : public Pipeline
{
public:
	VulkanPipeline(const PipelineSpecification& specification);
	virtual ~VulkanPipeline() override;

	virtual PipelineSpecification& GetSpecification() override { return m_specification; };
	virtual const PipelineSpecification& GetSpecification() const override { return m_specification; };

	virtual void Invalidate() override;
	void RT_Invalidate();
	virtual void SetUniformBuffer(ref<UniformBuffer> uniform_buffer, uint32_t binding, uint32_t set = 0) override;
	void RT_SetUniformBuffer(ref<UniformBuffer> uniform_buffer, uint32_t binding, uint32_t set = 0);

	VkPipeline GetVkPipeline() { return m_vk_pipeline; }
	VkPipelineLayout GetVkPipelineLayout() { return m_vk_pipeline_layout; }
	VkDescriptorSet GetVkDescriptorSet(uint32_t set = 0)
	{
		KB_CORE_ASSERT(set < m_descriptor_sets.descriptor_sets.size(), "out of bounds!");
		return m_descriptor_sets.descriptor_sets[set];
	}

	const std::vector<VkDescriptorSet>& GetDescriptorSets() { return m_descriptor_sets.descriptor_sets; }

private:
	PipelineSpecification m_specification;

	VkPipeline m_vk_pipeline = nullptr;
	VkPipelineLayout m_vk_pipeline_layout = nullptr;
	// #TODO pipeline caching
	VulkanShader::ShaderMaterialDescriptorSet m_descriptor_sets;
};
} // end namespace kb

#endif
