#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_PIPELINE_H
#define KABLUNK_PLATFORM_VULKAN_PIPELINE_H

#include "Kablunk/Renderer/Pipeline.h"

#include "Platform/Vulkan/VulkanShader.h"

#include <vulkan/vulkan.h>

namespace kb::render
{ // start namespace kb
class VulkanPipeline final : public Pipeline
{
public:
	VulkanPipeline(const PipelineSpecification& specification);
	~VulkanPipeline() override;

	PipelineSpecification& GetSpecification() override { return m_specification; }
	const PipelineSpecification& GetSpecification() const override { return m_specification; }

	void Invalidate() override;
    arc<Shader> get_shader() const noexcept override { return m_specification.shader; }

	void RT_Invalidate();

	VkPipeline get_vk_pipeline() const { return m_vk_pipeline; }
	VkPipelineLayout get_vk_pipeline_layout() const { return m_vk_pipeline_layout; }

private:
	PipelineSpecification m_specification;

	VkPipeline m_vk_pipeline = nullptr;
	VkPipelineLayout m_vk_pipeline_layout = nullptr;
    VkPipelineCache m_vk_pipeline_cache = nullptr;
};
} // end namespace kb

#endif
