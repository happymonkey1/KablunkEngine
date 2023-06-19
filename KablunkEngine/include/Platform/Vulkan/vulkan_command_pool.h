#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_COMMAND_POOL_H
#define KABLUNK_PLATFORM_VULKAN_COMMAND_POOL_H

#include "Kablunk/Core/RefCounting.h"

#include <vulkan/vulkan.h>

namespace kb::vk
{ // start namespace kb::vk

enum class command_buffer_type_t : uint8_t
{
    graphics = 0,
    compute
};

class command_pool : public Kablunk::RefCounted
{
public:
    command_pool();
    ~command_pool();

    VkCommandBuffer allocate_command_buffer(bool p_begin, bool p_compute = false);
    void flush_command_buffer(VkCommandBuffer p_vk_command_buffer);
    void flush_command_buffer(VkCommandBuffer p_vk_command_buffer, VkQueue p_vk_queue, command_buffer_type_t p_command_buffer_type);

    VkCommandPool get_vk_graphics_command_pool() const { return m_vk_graphics_command_pool; }
    VkCommandPool get_vk_compute_command_pool() const { return m_vk_compute_command_pool; }
private:
    VkCommandPool m_vk_graphics_command_pool = nullptr;
    VkCommandPool m_vk_compute_command_pool = nullptr;
};

} // end namespace kb::vk

#endif
