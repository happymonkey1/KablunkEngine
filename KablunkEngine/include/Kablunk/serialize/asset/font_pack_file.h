#pragma once

#include "Kablunk/Core/Core.h"
#include <Kablunk/Renderer/Font/font.h>

namespace kb::asset
{ // start namespace kb::asset

struct font_pack_header
{
    static constexpr u32 k_version = 1;
    static constexpr char k_magic_header[4] = { 'k', 'b', 'f', 'p' };
    char m_magic_header[4] = { k_magic_header[0], k_magic_header[1], k_magic_header[2], k_magic_header[3] };
    u32 m_version = k_version;
    // number of font atlases stored in the pack
    // #TODO could be u8 or u16 if we *really* want to save a couple bytes
    u64 m_atlas_count = 0;

    auto is_valid() const noexcept -> bool
    {
        return m_magic_header[0] == k_magic_header[0] &&
            m_magic_header[1] == k_magic_header[1] &&
            m_magic_header[2] == k_magic_header[2] &&
            m_magic_header[3] == k_magic_header[3];
    }
};

struct font_atlas_header
{
    static constexpr size_t k_name_size = 64ull;
    u32 m_image_type = 0;
    u32 m_width = 0;
    u32 m_height = 0;
    char m_font_name[k_name_size]{ 0 };
    // checksum on atlas content
    u128 m_checksum{};
};

struct font_pack_file
{
    // header for the font pack
    font_pack_header m_header{};
    // headers for individual atlases
    std::vector<font_atlas_header> m_font_atlas_headers{};
    // atlas data
    std::vector<ref<render::font>> m_fonts{};
};

} // end namespace kb::asset
