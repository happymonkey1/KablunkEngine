#include "kablunkpch.h"
#include "Platform/Vulkan/vulkan_command_pool.h"

#include "Platform/Vulkan/VulkanContext.h"

namespace kb::vk
{ // start namespace kb::vk



command_pool::command_pool()
{
    auto device = Kablunk::VulkanContext::Get()->GetDevice();
    VkDevice vk_device = device->GetVkDevice();

    VkCommandPoolCreateInfo vk_command_pool_create_info = {};
    vk_command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vk_command_pool_create_info.queueFamilyIndex = device->GetPhysicalDevice()->GetQueueFamilyIndices().Graphics_family.value();
    vk_command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if(vkCreateCommandPool(vk_device, &vk_command_pool_create_info, nullptr, &m_vk_graphics_command_pool) != VK_SUCCESS)
        KB_CORE_ASSERT(false, "[vk::command_pool]: failed to create vulkan graphics command pool!");

    vk_command_pool_create_info.queueFamilyIndex = device->GetPhysicalDevice()->GetQueueFamilyIndices().m_compute_family.value();
    if(vkCreateCommandPool(vk_device, &vk_command_pool_create_info, nullptr, &m_vk_compute_command_pool) != VK_SUCCESS)
        KB_CORE_ASSERT(false, "[vk::command_pool]: failed to create vulkan compute command pool")
}

command_pool::~command_pool()
{
    VkDevice vk_device = Kablunk::VulkanContext::Get()->GetDevice()->GetVkDevice();

    // destroy graphics
    vkDestroyCommandPool(vk_device, m_vk_graphics_command_pool, nullptr);
    // destroy compute
    vkDestroyCommandPool(vk_device, m_vk_compute_command_pool, nullptr);
}

VkCommandBuffer command_pool::allocate_command_buffer(bool p_begin, bool p_compute /*= false*/)
{
    VkDevice vk_device = Kablunk::VulkanContext::Get()->GetDevice()->GetVkDevice();

    VkCommandBuffer vk_command_buffer;

    VkCommandBufferAllocateInfo vk_command_buffer_alloc_info = {};
    vk_command_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vk_command_buffer_alloc_info.commandPool = !p_compute ? m_vk_graphics_command_pool: m_vk_compute_command_pool;
    vk_command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk_command_buffer_alloc_info.commandBufferCount = 1;

    if(vkAllocateCommandBuffers(vk_device, &vk_command_buffer_alloc_info, &vk_command_buffer))
        KB_CORE_ASSERT(false, "[vk::command_pool]: failed to allocate command buffer!")

    if (p_begin)
    {
        VkCommandBufferBeginInfo vk_command_buffer_begin_info{};
        vk_command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if(vkBeginCommandBuffer(vk_command_buffer, &vk_command_buffer_begin_info) != VK_SUCCESS)
            KB_CORE_ASSERT(false, "[vk::command_pool]: failed to begin command buffer!")
    }

    return vk_command_buffer;
}

void command_pool::flush_command_buffer(VkCommandBuffer p_vk_command_buffer)
{
    auto device = Kablunk::VulkanContext::Get()->GetDevice();
    flush_command_buffer(p_vk_command_buffer, device->GetGraphicsQueue(), command_buffer_type_t::graphics);
}

void command_pool::flush_command_buffer(VkCommandBuffer p_vk_command_buffer, VkQueue p_vk_queue, command_buffer_type_t p_command_buffer_type)
{
    VkDevice vk_device = Kablunk::VulkanContext::Get()->GetDevice()->GetVkDevice();

    const uint64_t k_default_fence_timeout = 100'000'000'000;

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
    if(vkCreateFence(vk_device, &fenceCreateInfo, nullptr, &vk_fence) != VK_SUCCESS)
        KB_CORE_ASSERT(false, "[vk::command_pool]: failed to create vkFence!");

    {
        static std::mutex lock_mutex;
        std::scoped_lock<std::mutex> lock(lock_mutex);

        // Submit to the queue
        if(vkQueueSubmit(p_vk_queue, 1, &vk_submit_info, vk_fence) != VK_SUCCESS)
            KB_CORE_ASSERT(false, "[vk::command_pool]: failed to submit vk queue!");
    }
    // wait for the fence to signal that command buffer has finished executing
    if(vkWaitForFences(vk_device, 1, &vk_fence, VK_TRUE, k_default_fence_timeout) != VK_SUCCESS)
        KB_CORE_ASSERT(false, "[vk::command_pool]: failed to wait for fence!");

    vkDestroyFence(vk_device, vk_fence, nullptr);

    auto vk_command_pool = p_command_buffer_type == command_buffer_type_t::graphics ? m_vk_graphics_command_pool : m_vk_compute_command_pool;
    vkFreeCommandBuffers(vk_device, vk_command_pool, 1, &p_vk_command_buffer);
}

} // end namespace kb::vk
