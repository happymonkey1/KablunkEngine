#include "kablunkpch.h"
#include "kablunk/renderer/backend/vulkan/vulkan_command_pool.h"

#include "kablunk/renderer/backend/vulkan/vulkan_context.h"
#include "kablunk/renderer/backend/vulkan/vulkan_logical_device.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

vulkan_command_pool::vulkan_command_pool(const VkDevice p_vk_device, const u32 p_graphics_family_index, const u32 p_compute_family_index)
    : m_vk_device{ p_vk_device }
{
    VkCommandPoolCreateInfo vk_command_pool_create_info = {};
    vk_command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vk_command_pool_create_info.queueFamilyIndex = p_graphics_family_index;
    vk_command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if(vkCreateCommandPool(m_vk_device, &vk_command_pool_create_info, nullptr, &m_vk_graphics_command_pool) != VK_SUCCESS)
        KB_CORE_ASSERT(false, "[vk::command_pool]: failed to create vulkan graphics command pool!");

    vk_command_pool_create_info.queueFamilyIndex = p_compute_family_index;
    if (vkCreateCommandPool(m_vk_device, &vk_command_pool_create_info, nullptr, &m_vk_compute_command_pool) != VK_SUCCESS)
        KB_CORE_ASSERT(false, "[vk::command_pool]: failed to create vulkan compute command pool");
}

vulkan_command_pool::~vulkan_command_pool() noexcept
{
    if (!m_vk_device)
        return;

    KB_CORE_INFO("[vk::command_pool]: destroying graphics command pool {}", static_cast<void*>(m_vk_graphics_command_pool));
    KB_CORE_INFO("[vk::command_pool]: destroying compute command pool {}", static_cast<void*>(m_vk_compute_command_pool));

    // destroy graphics
    vkDestroyCommandPool(m_vk_device, m_vk_graphics_command_pool, nullptr);
    // destroy compute
    vkDestroyCommandPool(m_vk_device, m_vk_compute_command_pool, nullptr);
}

vulkan_command_pool::vulkan_command_pool(vulkan_command_pool&& p_other) noexcept
    : m_vk_device{ p_other.m_vk_device }, m_vk_graphics_command_pool{ p_other.m_vk_graphics_command_pool },
    m_vk_compute_command_pool{ p_other.m_vk_compute_command_pool }
{
    p_other.m_vk_device = nullptr;
    p_other.m_vk_graphics_command_pool = nullptr;
    p_other.m_vk_compute_command_pool = nullptr;
}

auto vulkan_command_pool::operator=(vulkan_command_pool&& p_other) noexcept -> vulkan_command_pool&
{
    m_vk_device = p_other.m_vk_device;
    m_vk_graphics_command_pool = p_other.m_vk_graphics_command_pool;
    m_vk_compute_command_pool = p_other.m_vk_compute_command_pool;

    p_other.m_vk_device = nullptr;
    p_other.m_vk_graphics_command_pool = nullptr;
    p_other.m_vk_compute_command_pool = nullptr;

    return *this;
}

VkCommandBuffer vulkan_command_pool::allocate_command_buffer(bool p_begin, bool p_compute /*= false*/) const
{
    VkCommandBuffer vk_command_buffer;

    VkCommandBufferAllocateInfo vk_command_buffer_alloc_info = {};
    vk_command_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vk_command_buffer_alloc_info.commandPool = !p_compute ? m_vk_graphics_command_pool : m_vk_compute_command_pool;
    vk_command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk_command_buffer_alloc_info.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(m_vk_device, &vk_command_buffer_alloc_info, &vk_command_buffer))
        KB_CORE_ASSERT(false, "[vk::command_pool]: failed to allocate command buffer!");

    if (p_begin)
    {
        VkCommandBufferBeginInfo vk_command_buffer_begin_info{};
        vk_command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(vk_command_buffer, &vk_command_buffer_begin_info) != VK_SUCCESS)
            KB_CORE_ASSERT(false, "[vk::command_pool]: failed to begin command buffer!");
    }

    return vk_command_buffer;
}

#if 0
void command_pool::flush_command_buffer(VkCommandBuffer p_vk_command_buffer)
{
    flush_command_buffer(p_vk_command_buffer, m_vk_device->get_vk_graphics_queue(), command_buffer_type_t::graphics);
}
#endif

void vulkan_command_pool::flush_command_buffer(VkCommandBuffer p_vk_command_buffer, VkQueue p_vk_queue, command_buffer_type_t p_command_buffer_type) const
{
    constexpr uint64_t k_default_fence_timeout = 100'000'000'000;

    KB_CORE_ASSERT(p_vk_command_buffer != VK_NULL_HANDLE, "[vk::command_pool]: command buffer is null?");

    if(vkEndCommandBuffer(p_vk_command_buffer) != VK_SUCCESS)
        KB_CORE_ASSERT(false, "[vk::command_pool]: failed to end command buffer!");

    VkSubmitInfo vk_submit_info = {};
    vk_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vk_submit_info.commandBufferCount = 1;
    vk_submit_info.pCommandBuffers = &p_vk_command_buffer;

    // create fence to ensure that the command buffer has finished executing
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;

    VkFence vk_fence;
    if(vkCreateFence(m_vk_device, &fenceCreateInfo, nullptr, &vk_fence) != VK_SUCCESS)
        KB_CORE_ASSERT(false, "[vk::command_pool]: failed to create vkFence!");

    {
        static std::mutex lock_mutex;
        std::scoped_lock<std::mutex> lock(lock_mutex);

        // Submit to the queue
        if(vkQueueSubmit(p_vk_queue, 1, &vk_submit_info, vk_fence) != VK_SUCCESS)
            KB_CORE_ASSERT(false, "[vk::command_pool]: failed to submit vk queue!");
    }
    // wait for the fence to signal that command buffer has finished executing
    if(vkWaitForFences(m_vk_device, 1, &vk_fence, VK_TRUE, k_default_fence_timeout) != VK_SUCCESS)
        KB_CORE_ASSERT(false, "[vk::command_pool]: failed to wait for fence!");

    vkDestroyFence(m_vk_device, vk_fence, nullptr);

    const auto vk_command_pool = p_command_buffer_type == command_buffer_type_t::graphics ? m_vk_graphics_command_pool : m_vk_compute_command_pool;
    vkFreeCommandBuffers(m_vk_device, vk_command_pool, 1, &p_vk_command_buffer);
}

} // end namespace kb::render::backend::vk
