#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_PIPELINE_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_PIPELINE_H

#include "Kablunk/renderer/backend/pipeline.h"
#include "kablunk/renderer/backend/vulkan/vulkan_shader.h"

#include <vulkan/vulkan.h>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

class vulkan_pipeline final : public pipeline
{
public:
	vulkan_pipeline(VkDevice p_vk_device, const pipeline_specification_t& specification);
	~vulkan_pipeline() override;

	pipeline_specification_t& get_specification() override { return m_specification; }
	const pipeline_specification_t& get_specification() const override { return m_specification; }

	void invalidate() override;
    arc<shader> get_shader() const noexcept override { return m_specification.shader; }

	void RT_Invalidate();

	VkPipeline get_vk_pipeline() const { return m_vk_pipeline; }
	VkPipelineLayout get_vk_pipeline_layout() const { return m_vk_pipeline_layout; }

private:
    VkDevice m_vk_device = nullptr;
	pipeline_specification_t m_specification;

	VkPipeline m_vk_pipeline = nullptr;
	VkPipelineLayout m_vk_pipeline_layout = nullptr;
    VkPipelineCache m_vk_pipeline_cache = nullptr;
};

} // end namespace kb::render::backend::vk

#endif
