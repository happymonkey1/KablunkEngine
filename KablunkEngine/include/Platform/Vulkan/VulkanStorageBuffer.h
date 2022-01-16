#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_STORAGE_BUFFER_H
#define KABLUNK_PLATFORM_VULKAN_STORAGE_BUFFER_H

#include "Kablunk/Renderer/StorageBuffer.h"

#include "Platform/Vulkan/VulkanAllocator.h"

#include <vulkan/vulkan.h>

namespace Kablunk
{
	class VulkanStorageBuffer : public StorageBuffer 
	{
	public:
		VulkanStorageBuffer(size_t size, uint32_t binding);
		virtual ~VulkanStorageBuffer();

		virtual void SetData(const void* data, size_t size, uint32_t offset = 0) override;
		virtual void RT_SetData(const void* data, size_t size, uint32_t offset = 0) override;
		virtual void Resize(size_t new_size) override;

		virtual uint32_t GetBinding() { return m_binding; };

		const VkDescriptorBufferInfo& GetVkDescriptorInfo() const { return m_vk_descriptor_info; }
	private:
		void Release();
		void RT_Invalidate();
	private:
		VmaAllocation m_vk_memory_allocation;
		VkBuffer m_vk_buffer{};

		VkDescriptorBufferInfo m_vk_descriptor_info{};

		size_t m_size;
		uint32_t m_binding;

		std::string m_name;
		VkShaderStageFlagBits m_vk_shader_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;

		uint8_t* m_local_storage = nullptr; // #TODO change to buffer;
	};
}

#endif
