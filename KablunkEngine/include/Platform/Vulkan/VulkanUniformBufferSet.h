#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_UNIFORM_BUFFER_SET_H
#define KABLUNK_PLATFORM_VULKAN_UNIFORM_BUFFER_SET_H

#include "Kablunk/Renderer/UniformBufferSet.h"

#include <map>

namespace Kablunk
{
	class VulkanUniformBufferSet : public UniformBufferSet
	{
	public:
		VulkanUniformBufferSet(uint32_t frames);
		virtual ~VulkanUniformBufferSet() = default;

		virtual void Create(uint32_t size, uint32_t binding) override;

		virtual ref<UniformBuffer> Get(uint32_t binding, uint32_t set = 0, uint32_t frame = 0) override;
		virtual void Set(ref<UniformBuffer> uniform_buffer, uint32_t set = 0, uint32_t frame = 0) override;
	private:
		uint32_t m_frames;
		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, ref<UniformBuffer>>>> m_uniform_buffers;
	};
}

#endif
