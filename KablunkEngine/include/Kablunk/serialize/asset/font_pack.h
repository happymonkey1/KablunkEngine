#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/serialize/asset/font_pack_file.h"

#include <filesystem>

namespace kb::asset
{ // start namespace kb::asset

class font_pack
{
public:
    font_pack() noexcept = default;
    explicit font_pack(std::filesystem::path p_filepath);
    ~font_pack() noexcept = default;

    // add a font to the font pack
    auto add_font(const ref<render::font>& p_font_asset) noexcept -> void;
    // retrieve all font assets from the font pack
    [[nodiscard]] auto get_font_assets() const noexcept -> const std::vector<ref<render::font>>& { return m_font_pack.m_fonts; }

    // load a serialized font asset pack into memory
    [[nodiscard]] static auto load(const std::filesystem::path& p_filepath) noexcept -> std::unique_ptr<font_pack>;
    // save a serialized font asset pack to disk
    auto save() const noexcept -> void;
private:
    // serialize an individual font
    static auto serialize_font(
        u8*& p_cursor,
        const ref<render::font>& p_font_asset
    ) noexcept -> void;
    // deserialize an individual font
    auto deserialize_font(
        u8*& p_cursor,
        size_t p_atlas_index
    ) noexcept -> void;
    // reserve space in dynamic arrays
    auto init_reserve_space() noexcept -> void;
private:
    // relative path to the font pack
    std::filesystem::path m_path{};
    // in-memory representation of the font pack
    font_pack_file m_font_pack{};
    // set of asset ids already included in the pack
    std::unordered_set<asset::asset_id_t> m_font_ids{};
};

} // end namespace kb::asset
