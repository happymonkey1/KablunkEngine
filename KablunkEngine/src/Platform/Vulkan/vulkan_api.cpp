#include "kablunkpch.h"
#include "Platform/Vulkan/vulkan_api.h"

#include "Platform/Vulkan/VulkanContext.h"

namespace kb::vk
{ // start namespace kb::vk

auto get_descriptor_set_alloc_info(
    const VkDescriptorSetLayout* p_layouts,
    u32 p_count,
    VkDescriptorPool p_pool
) noexcept -> VkDescriptorSetAllocateInfo
{
    const VkDescriptorSetAllocateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = p_pool,
        .descriptorSetCount = p_count,
        .pSetLayouts = p_layouts
    };
    return create_info;
}

auto create_sampler(VkSamplerCreateInfo p_sampler_create_info) noexcept -> VkSampler
{
    const auto vk_device = VulkanContext::Get()->GetDevice()->GetVkDevice();

    VkSampler sampler;
    KB_VK_CHECK_RESULT(vkCreateSampler(vk_device, &p_sampler_create_info, nullptr, &sampler));

    // #TODO track allocations

    return sampler;
}

auto destroy_sampler(VkSampler p_sampler) noexcept -> void
{
    const auto vk_device = VulkanContext::Get()->GetDevice()->GetVkDevice();
    vkDestroySampler(vk_device, p_sampler, nullptr);

    // #TODO track de-allocation
}

} // end namespace kb::vk
