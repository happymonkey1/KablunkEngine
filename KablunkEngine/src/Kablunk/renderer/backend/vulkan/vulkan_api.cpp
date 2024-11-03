#include "kablunkpch.h"

#include "kablunk/renderer/backend/vulkan/vulkan_api.h"
#include "kablunk/renderer/backend/vulkan/vulkan_context.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

auto get_current_vk_device() noexcept -> VkDevice
{
    return Singleton<Renderer>::get().get_graphics_context().as<vulkan_context>()->get_device()->get_vk_device();
}

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

auto create_sampler(const VkSamplerCreateInfo& p_sampler_create_info) noexcept -> VkSampler
{
    VkSampler sampler;
    KB_VK_CHECK_RESULT(vkCreateSampler(get_current_vk_device(), &p_sampler_create_info, nullptr, &sampler));

    // #TODO track allocations

    return sampler;
}

auto destroy_sampler(VkSampler p_sampler) noexcept -> void
{
    vkDestroySampler(get_current_vk_device(), p_sampler, nullptr);

    // #TODO track de-allocation
}

} // end namespace kb::render::backend::vk
