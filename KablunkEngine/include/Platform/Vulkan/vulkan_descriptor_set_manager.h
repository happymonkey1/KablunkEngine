#pragma once

#include <set>
#include <vulkan/vulkan_core.h>

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Renderer/StorageBuffer.h"
#include "Kablunk/Renderer/StorageBufferSet.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/UniformBuffer.h"
#include "Kablunk/Renderer/UniformBufferSet.h"
#include "Platform/Vulkan/VulkanShader.h"

#include <string_view>

namespace kb::render
{ // start namespace kb::render

enum class render_pass_resource_type_t : u16
{
    none = 0,
    uniform_buffer,
    uniform_buffer_set,
    storage_buffer,
    storage_buffer_set,
    texture_2d,
    // #TODO not supported right now...
    texture_3d,
    image_2d,
};

enum class render_pass_input_type_t : u16
{
    none = 0,
    uniform_buffer,
    storage_buffer,
    image_sampler_1d,
    image_sampler_2d,
    image_sample_3d,
    storage_image_1d,
    storage_image_2d,
    storage_image_3d,
};

struct render_pass_input
{
    render_pass_resource_type_t m_type = render_pass_resource_type_t::none;
    std::vector<ref<RefCounted>> m_input{};

    render_pass_input() noexcept = default;
    ~render_pass_input() noexcept = default;

    explicit render_pass_input(const ref<UniformBuffer>& p_uniform_buffer) noexcept
        : m_type{ render_pass_resource_type_t::uniform_buffer },
        m_input{ std::vector(1, p_uniform_buffer.As<RefCounted>()) }
    {
    }

    explicit render_pass_input(const ref<UniformBufferSet>& p_uniform_buffer_set) noexcept
        : m_type{ render_pass_resource_type_t::uniform_buffer_set },
        m_input{ std::vector(1, p_uniform_buffer_set.As<RefCounted>()) }
    {
    }

    explicit render_pass_input(const ref<StorageBuffer>& p_storage_buffer) noexcept
        : m_type{ render_pass_resource_type_t::storage_buffer },
        m_input{ std::vector(1, p_storage_buffer.As<RefCounted>()) }
    {
    }

    explicit render_pass_input(const ref<StorageBufferSet>& p_storage_buffer_set) noexcept
        : m_type{ render_pass_resource_type_t::storage_buffer_set },
        m_input{ std::vector(1, p_storage_buffer_set.As<RefCounted>()) }
    {
    }

    explicit render_pass_input(const ref<Texture2D>& p_texture) noexcept
        : m_type{ render_pass_resource_type_t::texture_2d },
        m_input{ std::vector(1, p_texture.As<RefCounted>()) }
    {
    }

    explicit render_pass_input(const ref<Image2D>& p_image) noexcept
        : m_type{ render_pass_resource_type_t::image_2d },
        m_input{ std::vector(1, p_image.As<RefCounted>()) }
    {
    }

    auto set(const ref<UniformBuffer>& p_uniform_buffer, u32 p_index = 0) noexcept -> void
    {
        m_type = render_pass_resource_type_t::uniform_buffer;
        m_input[p_index] = p_uniform_buffer;
    }

    auto set(const ref<UniformBufferSet>& p_storage_buffer_set, u32 p_index = 0) noexcept -> void
    {
        m_type = render_pass_resource_type_t::uniform_buffer;
        m_input[p_index] = p_storage_buffer_set;
    }

    auto set(const ref<StorageBuffer>& p_storage_buffer, u32 p_index = 0) noexcept -> void
    {
        m_type = render_pass_resource_type_t::uniform_buffer;
        m_input[p_index] = p_storage_buffer;
    }

    auto set(const ref<StorageBufferSet>& p_storage_buffer_set, u32 p_index = 0) noexcept -> void
    {
        m_type = render_pass_resource_type_t::uniform_buffer;
        m_input[p_index] = p_storage_buffer_set;
    }

    auto set(const ref<Texture2D>& p_texture, u32 p_index = 0) noexcept -> void
    {
        m_type = render_pass_resource_type_t::uniform_buffer;
        m_input[p_index] = p_texture;
    }

    auto set(const ref<Image2D>& p_image, u32 p_index = 0) noexcept -> void
    {
        m_type = render_pass_resource_type_t::uniform_buffer;
        m_input[p_index] = p_image;
    }
};

[[nodiscard]] static auto is_input_compatible(
    render_pass_resource_type_t p_type,
    VkDescriptorType p_vk_descriptor_type
) noexcept -> bool
{
    switch (p_vk_descriptor_type)
    {
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: [[fallthrough]];
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    {
        return p_type == render_pass_resource_type_t::texture_2d ||
            p_type == render_pass_resource_type_t::texture_3d ||
            p_type == render_pass_resource_type_t::image_2d;
    }
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        return p_type == render_pass_resource_type_t::image_2d;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        return p_type == render_pass_resource_type_t::uniform_buffer ||
            p_type == render_pass_resource_type_t::uniform_buffer_set;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        return p_type == render_pass_resource_type_t::storage_buffer ||
            p_type == render_pass_resource_type_t::storage_buffer_set;
    default:
        KB_CORE_ASSERT(false, "[vulkan_descriptor_set_manager]: Unhandled VkDescriptorType when trying to check resource type!");
    }

    return false;
}

[[nodiscard]] static auto vk_descriptor_type_to_render_pass_input_type(
    VkDescriptorType p_vk_descriptor_type
) noexcept -> render_pass_input_type_t
{
    switch (p_vk_descriptor_type)
    {
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: [[fallthrough]];
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        return render_pass_input_type_t::image_sampler_2d;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        return render_pass_input_type_t::storage_image_2d;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        return render_pass_input_type_t::uniform_buffer;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        return render_pass_input_type_t::storage_buffer;
    default:
        KB_CORE_ASSERT(false, "[vulkan_descriptor_set_manager]: Unhandled VkDescriptorType when trying to get render pass input type!");
    }

    return render_pass_input_type_t::none;
}

struct render_pass_input_declaration
{
    render_pass_input_type_t m_type = render_pass_input_type_t::none;
    u32 m_set = 0;
    u32 m_binding = 0;
    u32 m_count = 0;
    std::string m_name{};
};

struct descriptor_set_manager_specification
{
    ref<VulkanShader> m_shader{};
    std::string m_debug_name{};

    u32 m_start_set = 0;
    u32 m_end_set = 3;

    bool m_default_resources = false;
};

// input resources (map of set -> binding -> resource)
// Invalidated input resources attempt to be re-assigned on the next render pass
// this is useful for resources that may not exist at render pass creation,
// but will be present during actual rendering.
class descriptor_set_manager
{
public:
    descriptor_set_manager() noexcept = default;
    descriptor_set_manager(const descriptor_set_manager& p_other) noexcept;
    descriptor_set_manager(descriptor_set_manager&& p_other) noexcept;
    explicit descriptor_set_manager(const descriptor_set_manager_specification& p_spec) noexcept;
    ~descriptor_set_manager() noexcept;

    auto set_input(std::string_view p_name, const ref<UniformBuffer>& p_uniform_buffer) noexcept -> void;
    auto set_input(std::string_view p_name, const ref<UniformBufferSet>& p_uniform_buffer_set) noexcept -> void;
    auto set_input(std::string_view p_name, const ref<StorageBuffer>& p_storage_buffer) noexcept -> void;
    auto set_input(std::string_view p_name, const ref<StorageBufferSet>& p_storage_buffer_set) noexcept -> void;
    auto set_input(std::string_view p_name, const ref<Texture2D>& p_texture_2d) noexcept -> void;
    auto set_input(std::string_view p_name, const ref<Image2D>& p_image_2d) noexcept -> void;

    template <typename T>
    ref<T> get_input(std::string_view p_name);

    auto is_invalidated(u32 p_set, u32 p_binding) const noexcept -> bool;
    auto validate() noexcept -> bool;
    auto bake() noexcept -> void;

    // #TODO can this be unordered set?
    auto has_buffer_sets() noexcept -> std::set<u32>;
    auto invalidate_and_update() noexcept -> void;

    auto get_descriptor_pool() const noexcept -> VkDescriptorPool { return m_descriptor_pool; }
    auto has_descriptor_sets() const noexcept -> bool;
    auto get_first_set_index() const noexcept -> u32;
    auto get_descriptor_sets() const noexcept -> const std::vector<VkDescriptorSet>&;
    auto is_input_valid(std::string_view p_name) const noexcept -> bool;
    auto get_input_declaration(std::string_view p_name) const noexcept -> const render_pass_input_declaration*;

    auto operator=(const descriptor_set_manager& p_other) noexcept -> descriptor_set_manager&;
    auto operator=(descriptor_set_manager&& p_other) noexcept -> descriptor_set_manager&;

private:
    auto init() noexcept -> void;

private:
    // #TODO do these need to be ordered maps?
    std::map<u32, std::map<u32, render_pass_input>> m_input_resources{};
    std::map<u32, std::map<u32, render_pass_input>> m_invalidated_input_resources{};
    std::map<std::string, render_pass_input_declaration> m_input_declarations{};
    // per frames in flight
    std::vector<std::vector<VkDescriptorSet>> m_descriptor_sets{};

    struct write_descriptor
    {
        VkWriteDescriptorSet m_write_descriptor_set{};
        std::vector<void*> m_resource_handles{};
    };

    std::vector<std::map<u32, std::map<u32, write_descriptor>>> m_write_descriptor_map{};

    descriptor_set_manager_specification m_specification{};
    VkDescriptorPool m_descriptor_pool = nullptr;
};

template <typename T>
ref<T> descriptor_set_manager::get_input(std::string_view p_name)
{
    if (const auto* decl = get_input_declaration(p_name))
        if (const auto set = m_input_resources.find(decl->m_binding); set != m_input_resources.end())
            if (const auto resource = set->second.find(decl->m_binding); resource != set->second.end())
                return resource->second.m_input[0].As<T>();

    return ref<T>{};
}

} // end namespace kb::render
