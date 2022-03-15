#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_UNIFORM_BUFFER_H
#define KABLUNK_PLATFORM_VULKAN_UNIFORM_BUFFER_H

#include "Kablunk/Renderer/UniformBuffer.h"
#include "Platform/Vulkan/VulkanAllocator.h"

#include <vulkan/vulkan.h>

namespace Kablunk
{
	class VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~VulkanUniformBuffer();

		virtual void SetData(const void* data, uint32_t size, uint32_t offest = 0) override;
		virtual void RT_SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
		virtual RendererID GetBinding() const override { return m_binding; }

		const VkDescriptorBufferInfo& GetDescriptorBufferInfo() const { return m_descriptor_info; }
	private:
		void RT_Invalidate();
		void Release();
	private:
		VkBuffer m_buffer;
		VkDescriptorBufferInfo m_descriptor_info{};
		VmaAllocation m_vk_allocation = nullptr;

		uint32_t m_size = 0;
		RendererID m_binding = 0;
		std::string m_name;
		VkShaderStageFlagBits m_shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;

		uint8_t* m_local_storage;
	};
}

#endif
