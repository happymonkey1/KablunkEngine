#pragma once

#include "Kablunk/renderer/backend/texture.h"

#include <vendor/VulkanMemoryAllocator/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

// TODO: make this `vulkan_texture_3d` and have vulkan_texture_cube be a `vulkan_texture_3d` with a default extent
class vulkan_texture_cube final : public texture_cube
{
public:
    vulkan_texture_cube(VkDevice p_vk_device, const texture_specification_t& p_specification, const void* p_data, size_t p_size);
    ~vulkan_texture_cube() noexcept override;

    image_format_t get_format() const noexcept override { return m_specification.m_format; }

    resource_descriptor_info_t get_descriptor_info() noexcept override { return &m_vk_descriptor_image_info; }
    u32 get_width() const override { return m_specification.m_width; }
    u32 get_height() const override { return m_specification.m_height; }
    void set_data(void* data, u32 size) override;
    uint64_t get_hash() const override { return reinterpret_cast<u64>(m_vk_image); }
    void bind(u32 slot) const override { KB_CORE_ASSERT(false, "[vulkan_texture_3d]: bind() is not implemented!"); }
    u32 get_mip_level_count() const noexcept override;
    std::pair<u32, u32> get_mip_size(u32 p_mip) const noexcept override;

    auto get_vk_descriptor_image_info() const noexcept -> const VkDescriptorImageInfo& { return m_vk_descriptor_image_info; }
    auto get_vk_descriptor_image_info() noexcept -> VkDescriptorImageInfo& { return m_vk_descriptor_image_info; }

    auto create_image_view_with_single_mip(u32 p_mip) noexcept -> VkImageView;
    auto generate_mips(bool p_read_only = false) noexcept -> void;

    // Free allocated memory and submit vulkan resources for deletion
    auto destroy() noexcept -> void;

private:
    auto invalidate() noexcept -> void;

private:
    texture_specification_t m_specification{};
    owning_buffer m_local_buffer{};

    VkDevice m_vk_device{};
    VmaAllocation m_vk_memory_allocation{};
    u64 m_gpu_allocation_size = 0ull;
    VkImage m_vk_image{ nullptr };
    VkDescriptorImageInfo m_vk_descriptor_image_info{};

    bool m_generated_mips = false;
};

} // end namespace kb::render::backend::vk
