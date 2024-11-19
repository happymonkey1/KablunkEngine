#pragma once

#include "Kablunk/Math/rectangle.h"
#include "Kablunk/renderer/texture_handle.h"
#include "Kablunk/renderer/backend/texture.h"
#include "Kablunk/renderer/virtual_texture.h"

#include <filesystem>
#include <array>

namespace kb::render
{ // start namespace kb::render

struct texture_atlas_create_props
{
    std::filesystem::path m_path = "";
    bool m_force_create = false;

    std::filesystem::path m_root_directory;
};

class texture_atlas : public backend::texture_2d
{
public:
    constexpr static size_t k_default_atlas_size = 4096ull;

public:
    texture_atlas() noexcept = default;
    ~texture_atlas() noexcept override = default;

    texture_atlas(const texture_atlas&) noexcept = delete;
    texture_atlas(texture_atlas&&) noexcept = default;

    [[nodiscard]] static auto create(

    ) noexcept -> arc<texture_atlas>;

    [[nodiscard]] auto get_filepath() const noexcept -> const std::filesystem::path& { return m_filepath; }

    [[nodiscard]] auto get_uv_map() const noexcept -> const unordered_flat_map<virtual_texture_handle, virtual_texture_t>& { return m_uv_map; }

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
        std::swap(a.m_uv_map, b.m_uv_map);
    }

private:
    struct image_data_t
    {
        inline static constexpr size_t k_bit_depth_data_size = sizeof(float);
        owning_buffer m_image_data{};
        u32 m_width = 0;
        u32 m_height = 0;
        virtual_texture_handle m_texture_handle;

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
    explicit texture_atlas(const texture_atlas_create_props& p_props) noexcept;
    explicit texture_atlas(std::initializer_list<arc<backend::texture_2d>> p_textures) noexcept;
    explicit texture_atlas(const std::vector<backend::texture_2d>& p_textures) noexcept;

    auto create_texture_atlas(const texture_atlas_create_props& p_props) noexcept -> void;
    auto add_image_to_atlas(
        const node_t* p_node,
        const image_data_t& p_image_data,
        owning_buffer& p_atlas_buffer
    ) noexcept -> void;
    [[nodiscard]] static auto load_image(const std::filesystem::path& p_path) noexcept -> image_data_t;
    auto calculate_uv_offsets(virtual_texture_handle p_texture_handle, const rect_i32& p_rect) noexcept -> void;
    // free binary tree
    static auto delete_tree(node_t* p_root) noexcept -> void;

private:
    std::filesystem::path m_filepath{};
    // width and height of the atlas
    u32 m_atlas_dimension = k_default_atlas_size;
    u32 m_sprite_count = 0ul;
    unordered_flat_map<virtual_texture_handle, virtual_texture_t> m_uv_map{};
};

} // end namespace kb::render
