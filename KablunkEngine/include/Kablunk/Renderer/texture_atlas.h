#pragma once

#include "Kablunk/Math/rectangle.h"
#include "Kablunk/Renderer/texture_handle.h"
#include "Kablunk/Renderer/Texture.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <array>

namespace kb::render
{ // start namespace kb::render

struct texture_atlas_create_props
{
    std::filesystem::path m_path = "";
    bool m_force_create = false;
    u32 m_sprite_width = 0ul;

    std::filesystem::path m_root_directory;
};

class texture_atlas : public RefCounted
{
public:
    constexpr static size_t k_default_atlas_size = 4096ull;

    struct virtual_texture_data_t
    {
        std::array<glm::vec2, 4> m_uvs;
        glm::uvec2 m_sprite_dimensions;
    };

public:
    texture_atlas() noexcept = default;
    ~texture_atlas() noexcept override = default;

    texture_atlas(const texture_atlas&) noexcept = delete;
    texture_atlas(texture_atlas&&) noexcept = default;

    explicit texture_atlas(const texture_atlas_create_props& p_props) noexcept;

    [[nodiscard]] auto get_filepath() const noexcept -> const std::filesystem::path& { return m_filepath; }

    auto set_texture_atlas(const arc<Texture2D>& p_texture) noexcept -> void { m_texture = p_texture; }
    [[nodiscard]] auto get_texture_atlas() const noexcept -> const arc<Texture2D>& { return m_texture; }

    [[nodiscard]] auto get_uv_map() const noexcept -> const unordered_flat_map<raw_texture_handle, virtual_texture_data_t>& { return m_uv_map; }

    auto operator=(const texture_atlas&) noexcept -> texture_atlas& = delete;

    auto operator=(texture_atlas&& p_other) noexcept -> texture_atlas&
    {
        swap(*this, p_other);
        return *this;
    }

    friend auto swap(texture_atlas& a, texture_atlas& b) noexcept -> void
    {
        std::swap(a.m_filepath, b.m_filepath);
        std::swap(a.m_atlas_dimension, b.m_atlas_dimension);
        std::swap(a.m_sprite_count, b.m_sprite_count);
        std::swap(a.m_texture, b.m_texture);
        std::swap(a.m_uv_map, b.m_uv_map);
    }

private:
    struct image_data_t
    {
        inline static constexpr size_t k_bit_depth_data_size = sizeof(float);
        owning_buffer m_image_data{};
        u32 m_width = 0;
        u32 m_height = 0;
        raw_texture_handle m_id;

        [[nodiscard]] constexpr auto is_valid() const noexcept -> bool
        {
            return m_width != 0 && m_height != 0 && m_image_data.get();
        }
    };

    struct node_t
    {
        node_t* m_left = nullptr;
        node_t* m_right = nullptr;
        rect_i32 m_rect{};
        u64 m_image_hash = 0ull;

        [[nodiscard]] auto insert(const image_data_t& p_image_data) noexcept -> node_t*;
        [[nodiscard]] auto is_leaf() const noexcept -> bool { return !m_left && !m_right; }
    };

private:
    auto create_texture_atlas(const texture_atlas_create_props& p_props) noexcept -> void;
    auto add_image_to_atlas(
        const node_t* p_node,
        const image_data_t& p_image_data,
        owning_buffer& p_atlas_buffer
    ) noexcept -> void;
    [[nodiscard]] static auto load_image(const std::filesystem::path& p_path) noexcept -> image_data_t;
    auto calculate_uv_offsets(raw_texture_handle p_id, const rect_i32& p_rect) noexcept -> void;
    // free binary tree
    static auto delete_tree(node_t* p_root) noexcept -> void;

private:
    std::filesystem::path m_filepath{};
    // width and height of the atlas
    u32 m_atlas_dimension = k_default_atlas_size;
    u32 m_sprite_count = 0ul;
    arc<Texture2D> m_texture{};
    unordered_flat_map<raw_texture_handle, virtual_texture_data_t> m_uv_map{};
};

} // end namespace kb::render
