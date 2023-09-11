#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanRenderPass.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Kablunk/Renderer/RenderCommand.h"

namespace kb
{

	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& specification)
		: m_specification{ specification }
	{

	}

	VulkanRenderPass::~VulkanRenderPass()
	{

	}

}
