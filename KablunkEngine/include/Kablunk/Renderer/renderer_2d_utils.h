#pragma once

#include "Kablunk/Math/bounding_box.h"
#include "Kablunk/Renderer/Font/FontAsset.h"

#include <glm/glm.hpp>

#include <string_view>


namespace kb::render
{ // start namespace kb::render

// #TODO optimize first pass implementation...
// #NOTE this should be the same logic as Renderer2D.cpp draw_text_string
// returns a bounding box { top_left_x, top_left_y, bottom_right_x, bottom_right_y }
inline auto calculate_text_bounds(
    const std::string_view p_text,
    const glm::vec2& p_pos,
    const u32 p_font_point,
    const ref<font_asset>& p_font_asset
) noexcept -> bounding_box
{
    f32 width = 0;
    f32 x = p_pos.x, y = p_pos.y, z = p_pos.x, w = p_pos.y;

    const auto& glyph_info_map = p_font_asset->get_glyph_rendering_map();
    for (size_t i = 0; i < p_text.size(); ++i)
    {
        const auto text_char = p_text[i];
        const auto& glyph_data = glyph_info_map.contains(text_char) ? glyph_info_map.at(text_char) : glyph_info_map.at('?');
        const f32 char_height = glyph_data.m_size.y;

        width += static_cast<f32>(glyph_data.m_advance);

        if (i == 0)
        {
            x += static_cast<f32>(glyph_data.m_x_off);
            y -= static_cast<f32>(glyph_data.m_y_off);
        }
        else if (i == p_text.size() - 1)
        {
            z += width;
            w += -static_cast<f32>(glyph_data.m_y_off) + char_height;
        }
    }

    return bounding_box{ x, y, z, w };
}

} // end namespace kb::render
