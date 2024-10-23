#pragma once

#include <set>
#include <vulkan/vulkan_core.h>

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/meta/tuple_traits.h"
#include "Kablunk/Renderer/StorageBuffer.h"
#include "Kablunk/Renderer/StorageBufferSet.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/uniform_buffer.h"
#include "Kablunk/Renderer/UniformBufferSet.h"
#include "Platform/Vulkan/VulkanShader.h"

#include <string_view>

namespace kb::render
{ // start namespace kb::render

namespace
{
// size of the render pass c string array
constexpr size_t k_render_pass_resource_type_name_size = 8ull;
// render pass resource c strings
// must keep size aligned to the number of `render_pass_resource_type_t` enums
const char* k_render_pass_resource_type_names[k_render_pass_resource_type_name_size]
{
    "none",
    "uniform_buffer",
    "storage_buffer",
    "storage_buffer_set",
    "texture_2d",
    "texture_3d",
    "image_2d",
};
}

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

constexpr auto render_pass_resource_type_to_string(
    render_pass_resource_type_t p_type
) noexcept -> std::string_view
{
    const auto index = static_cast<std::underlying_type_t<render_pass_resource_type_t>>(p_type);
    KB_CORE_ASSERT(
        index < k_render_pass_resource_type_name_size,
        "[render_pass_resource_type_to_string]: Index out of bounds!"
    );

    return k_render_pass_resource_type_names[index];
}


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
    std::vector<arc<RefCounted>> m_input{};

    render_pass_input() noexcept = default;
    ~render_pass_input() noexcept = default;

    explicit render_pass_input(const arc<uniform_buffer>& p_uniform_buffer) noexcept
        : m_type{ render_pass_resource_type_t::uniform_buffer },
        m_input{ std::vector(1, p_uniform_buffer.As<RefCounted>()) }
    {
    }

    explicit render_pass_input(const arc<UniformBufferSet>& p_uniform_buffer_set) noexcept
        : m_type{ render_pass_resource_type_t::uniform_buffer_set },
        m_input{ std::vector(1, p_uniform_buffer_set.As<RefCounted>()) }
    {
    }

    explicit render_pass_input(const arc<StorageBuffer>& p_storage_buffer) noexcept
        : m_type{ render_pass_resource_type_t::storage_buffer },
        m_input{ std::vector(1, p_storage_buffer.As<RefCounted>()) }
    {
    }

    explicit render_pass_input(const arc<StorageBufferSet>& p_storage_buffer_set) noexcept
        : m_type{ render_pass_resource_type_t::storage_buffer_set },
        m_input{ std::vector(1, p_storage_buffer_set.As<RefCounted>()) }
    {
    }

    explicit render_pass_input(const arc<Texture2D>& p_texture) noexcept
        : m_type{ render_pass_resource_type_t::texture_2d },
        m_input{ std::vector(1, p_texture.As<RefCounted>()) }
    {
    }

    explicit render_pass_input(const arc<Image2D>& p_image) noexcept
        : m_type{ render_pass_resource_type_t::image_2d },
        m_input{ std::vector(1, p_image.As<RefCounted>()) }
    {
    }

    auto set(const arc<uniform_buffer>& p_uniform_buffer, u32 p_index = 0) noexcept -> void
    {
        m_type = render_pass_resource_type_t::uniform_buffer;
        m_input[p_index] = arc<RefCounted>{ p_uniform_buffer };
    }

    auto set(const arc<UniformBufferSet>& p_storage_buffer_set, u32 p_index = 0) noexcept -> void
    {
        m_type = render_pass_resource_type_t::uniform_buffer_set;
        m_input[p_index] = arc<RefCounted>{ p_storage_buffer_set };
    }

    auto set(const arc<StorageBuffer>& p_storage_buffer, u32 p_index = 0) noexcept -> void
    {
        m_type = render_pass_resource_type_t::storage_buffer;
        m_input[p_index] = arc<RefCounted>{ p_storage_buffer };
    }

    auto set(const arc<StorageBufferSet>& p_storage_buffer_set, u32 p_index = 0) noexcept -> void
    {
        m_type = render_pass_resource_type_t::storage_buffer_set;
        m_input[p_index] = arc<RefCounted>{ p_storage_buffer_set };
    }

    auto set(const arc<Texture2D>& p_texture, u32 p_index = 0) noexcept -> void
    {
        m_type = render_pass_resource_type_t::texture_2d;
        m_input[p_index] = arc<RefCounted>{ p_texture };
    }

    auto set(const arc<Image2D>& p_image, u32 p_index = 0) noexcept -> void
    {
        m_type = render_pass_resource_type_t::image_2d;
        m_input[p_index] = arc<RefCounted>{ p_image };
    }

    auto set(const arc<image_view>& p_image, u32 p_index = 0) noexcept -> void
    {
        m_type = render_pass_resource_type_t::image_2d;
        m_input[p_index] = arc<RefCounted>{ p_image };
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
    arc<VulkanShader> m_shader{};
    std::string m_debug_name{};

    u32 m_start_set = 0;
    u32 m_end_set = 3;

    bool m_default_resources = false;
};

namespace details
{ // start namespace ::details
// list of valid input resources
using render_pass_input_types_tuple = std::tuple<
    uniform_buffer,
    UniformBufferSet,
    StorageBuffer,
    StorageBufferSet,
    Texture2D,
    Image2D,
    image_view
>;
} // end namespace ::details

namespace concepts
{ // start namespace concepts
template <typename T>
concept RenderPassInputT = std::is_same_v<
    std::true_type,
    typename meta::tuple_has_type<T, details::render_pass_input_types_tuple>::type
>;
#if 0
static_assert(
    std::is_same_v<
        std::true_type,
        meta::tuple_has_type<Texture2D, details::render_pass_input_types_tuple>::type
    >
);
#endif
} // end namespace concepts


// input resources (map of set -> binding -> resource)
// Invalidated input resources attempt to be re-assigned on the next render pass
// this is useful for resources that may not exist at render pass creation,
// but will be present during actual rendering.
class vulkan_descriptor_set_manager
{
public:
    vulkan_descriptor_set_manager() noexcept = default;
    vulkan_descriptor_set_manager(const vulkan_descriptor_set_manager& p_other) noexcept;
    vulkan_descriptor_set_manager(vulkan_descriptor_set_manager&& p_other) noexcept;
    explicit vulkan_descriptor_set_manager(descriptor_set_manager_specification p_spec) noexcept;
    ~vulkan_descriptor_set_manager() noexcept = default;

    template <concepts::RenderPassInputT T>
    auto set_input(
        std::string_view p_name,
        const arc<T>& p_resource,
        const u32 p_index = 0
    ) noexcept -> vulkan_descriptor_set_manager&
    {
        return set_input_impl(p_name, p_resource, p_index);
    }

    template <typename T>
    arc<T> get_input(std::string_view p_name);

    // check if descriptor set at (set, binding) is invalidated
    auto is_invalidated(u32 p_set, u32 p_binding) const noexcept -> bool;
    // validate render pass inputs
    auto validate() noexcept -> bool;
    // validate render pass inputs and bind inputs as either write or invalidated write descriptors
    auto bake() noexcept -> void;

    // get a set of uniform buffer and storage buffer sets
    auto has_buffer_sets() noexcept -> std::set<u32>;
    // invalidate and update resources that are not in the write descriptor map
    auto rt_invalidate_and_update() noexcept -> void;

    auto get_descriptor_pool() const noexcept -> VkDescriptorPool { return m_descriptor_pool; }
    auto has_descriptor_sets() const noexcept -> bool;
    auto get_first_set_index() const noexcept -> std::optional<u32>;
    auto get_descriptor_sets(u32 frame_index) const noexcept -> const std::vector<VkDescriptorSet>&;
    auto is_input_valid(std::string_view p_name) const noexcept -> bool;
    auto get_input_declaration(std::string_view p_name) const noexcept -> const render_pass_input_declaration*;

    auto get_input_declarations() const noexcept -> const std::map<std::string, render_pass_input_declaration>&
    {
        return m_input_declarations;
    }

    auto get_input_declarations() noexcept -> std::map<std::string, render_pass_input_declaration>&
    {
        return m_input_declarations;
    }

    auto operator=(const vulkan_descriptor_set_manager& p_other) noexcept -> vulkan_descriptor_set_manager&;
    auto operator=(vulkan_descriptor_set_manager&& p_other) noexcept -> vulkan_descriptor_set_manager&;

private:
    auto init() noexcept -> void;

    template <typename T>
    auto set_input_impl(
        std::string_view p_name,
        const arc<T>& p_resource,
        u32 p_index = 0
    ) noexcept -> vulkan_descriptor_set_manager&;

private:
    // map of render pass inputs
    std::map<u32, std::map<u32, render_pass_input>> m_input_resources{};
    // invalidated per frame resources to be updated
    std::map<u32, std::map<u32, render_pass_input>> m_invalidated_input_resources{};
    // map of render pass inputs to their input decl
    std::map<std::string, render_pass_input_declaration> m_input_declarations{};
    // per frames in flight vulkan descriptor sets
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

// specialization for Texture2D to set at a specific index
template <>
inline auto vulkan_descriptor_set_manager::set_input_impl(
    std::string_view p_name,
    const arc<Texture2D>& p_resource,
    [[maybe_unused]] u32 p_index /* = 0 */
) noexcept -> vulkan_descriptor_set_manager&
{
    if (const auto* decl = get_input_declaration(p_name))
        m_input_resources.at(decl->m_set).at(decl->m_binding).set(p_resource, p_index);
    else
    {
        log::core::warn(
            log::logger_tag_t::renderer,
            "[Render Pass {}]: Input {} not found!",
            m_specification.m_debug_name,
            p_name
        );
    }

    return *this;
}

template <typename T>
auto vulkan_descriptor_set_manager::set_input_impl(
    std::string_view p_name,
    const arc<T>& p_resource,
    [[maybe_unused]] const u32 p_index /* = 0 */
) noexcept -> vulkan_descriptor_set_manager&
{
    if (const auto* decl = get_input_declaration(p_name))
        m_input_resources.at(decl->m_set).at(decl->m_binding).set(p_resource);
    else
    {
        log::core::warn(
            log::logger_tag_t::renderer,
            "[Render Pass {}]: Input {} not found!",
            m_specification.m_debug_name,
            p_name
        );
    }

    return *this;
}

template <typename T>
arc<T> vulkan_descriptor_set_manager::get_input(std::string_view p_name)
{
    if (const auto* decl = get_input_declaration(p_name))
        if (const auto set = m_input_resources.find(decl->m_binding); set != m_input_resources.end())
            if (const auto resource = set->second.find(decl->m_binding); resource != set->second.end())
                return resource->second.m_input[0].As<T>();

    return arc<T>{};
}

} // end namespace kb::render
