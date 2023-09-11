#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanUniformBufferSet.h"

namespace Kablunk
{

	VulkanUniformBufferSet::VulkanUniformBufferSet(uint32_t frames)
		: m_frames{ frames }
	{

	}

	void VulkanUniformBufferSet::Create(uint32_t size, uint32_t binding)
	{
		for (uint32_t frame = 0; frame < m_frames; ++frame)
		{
			ref<UniformBuffer> uniform_buffer = UniformBuffer::Create(size, binding);
			Set(uniform_buffer, 0, frame); // #TODO only one uniform buffer set supported!
		}
	}

	ref<UniformBuffer> VulkanUniformBufferSet::Get(uint32_t binding, uint32_t set /*= 0*/, uint32_t frame /*= 0*/)
	{
		KB_CORE_ASSERT(m_uniform_buffers.find(frame) != m_uniform_buffers.end(), "no uniform buffer found!");
		KB_CORE_ASSERT(m_uniform_buffers.at(frame).find(set) != m_uniform_buffers.at(frame).end(), "no uniform buffer set found!");
		KB_CORE_ASSERT(m_uniform_buffers.at(frame).at(set).find(binding) != m_uniform_buffers.at(frame).at(set).end(), "no uniform buffer set binding found!");

		return m_uniform_buffers.at(frame).at(set).at(binding);
	}

	void VulkanUniformBufferSet::Set(ref<UniformBuffer> uniform_buffer, uint32_t set /*= 0*/, uint32_t frame /*= 0*/)
	{
		m_uniform_buffers[frame][set][uniform_buffer->GetBinding()] = uniform_buffer;
	}

}
