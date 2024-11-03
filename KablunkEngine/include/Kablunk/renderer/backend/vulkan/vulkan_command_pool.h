#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_COMMAND_POOL_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_COMMAND_POOL_H

#include <vulkan/vulkan.h>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

enum class command_buffer_type_t : uint8_t
{
    graphics = 0,
    compute
};

class vulkan_command_pool
{
public:
    vulkan_command_pool() = delete;
    vulkan_command_pool(VkDevice p_vk_device, u32 p_graphics_family_index, u32 p_compute_family_index);
    ~vulkan_command_pool() noexcept;

    vulkan_command_pool(const vulkan_command_pool&) = delete;
    auto operator=(const vulkan_command_pool&) noexcept -> vulkan_command_pool& = delete;
    vulkan_command_pool(vulkan_command_pool&& p_other) noexcept;
    auto operator=(vulkan_command_pool&& p_other) noexcept -> vulkan_command_pool&;

    VkCommandBuffer allocate_command_buffer(bool p_begin, bool p_compute = false) const;
    void flush_command_buffer(
        VkCommandBuffer p_vk_command_buffer,
        VkQueue p_vk_queue,
        command_buffer_type_t p_command_buffer_type
    ) const;

    VkCommandPool get_vk_graphics_command_pool() const { return m_vk_graphics_command_pool; }
    VkCommandPool get_vk_compute_command_pool() const { return m_vk_compute_command_pool; }
private:
    VkDevice m_vk_device = nullptr;
    VkCommandPool m_vk_graphics_command_pool = nullptr;
    VkCommandPool m_vk_compute_command_pool = nullptr;
};

} // end namespace kb::render::backend::vk

#endif
