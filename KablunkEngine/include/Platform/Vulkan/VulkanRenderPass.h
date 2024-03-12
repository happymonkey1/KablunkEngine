#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_RENDER_PASS_H
#define KABLUNK_PLATFORM_VULKAN_RENDER_PASS_H

#include "Kablunk/Renderer/RenderPass.h"

#include <vulkan/vulkan.h>

namespace kb
{ // start namespace kb
class VulkanRenderPass final : public RenderPass
{
public:
	VulkanRenderPass(const RenderPassSpecification& specification);
	virtual ~VulkanRenderPass() override;

	virtual RenderPassSpecification& GetSpecification() override { return m_specification; }
	virtual const RenderPassSpecification& GetSpecification() const override { return m_specification; }
private:
	RenderPassSpecification m_specification;
};
} // end namespace kb

#endif
