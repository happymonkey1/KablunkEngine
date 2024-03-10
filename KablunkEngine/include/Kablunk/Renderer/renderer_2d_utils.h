#pragma once

#include "Kablunk/Math/bounding_box.h"
#include "Kablunk/Renderer/Font/FontAsset.h"
#include "Kablunk/Renderer/Font/FontManager.h"

#include <glm/glm.hpp>

#include <string_view>
#include <optional>


namespace kb::render
{ // start namespace kb::render

#pragma warning(disable : 4996)
// from https://stackoverflow.com/questions/31302506/stdu32string-conversion-to-from-stdstring-and-stdu16string
auto to_utf32_str(const std::string& p_str) noexcept -> std::u32string
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(p_str);
}
#pragma warning(default : 4996)

// #TODO optimize first pass implementation...
// #NOTE this should be the same logic as Renderer2D.cpp draw_text_string
// returns a bounding box { top_left_x, top_left_y, bottom_right_x, bottom_right_y }
inline auto calculate_text_bounds(
    const std::string& p_text,
    const glm::vec2& p_pos,
    const u32 p_font_point,
    const ref<font>& p_font_asset,
    const f32 p_max_width = 0.f,
    const f32 p_line_height_offset = 0.f,
    const f32 p_kerning_offset = 0.f,
    std::vector<i32>* p_next_lines = nullptr
) noexcept -> bounding_box
{
    const auto utf32_text = render::to_utf32_str(p_text);

    f32 width = 0, height = 0;
    f32 x = p_pos.x, y = p_pos.y, z = p_pos.x, w = p_pos.y;
    const f32 scale = static_cast<f32>(p_font_point) / static_cast<f32>(font_manager::k_load_font_point);

    const auto& font_geometry = p_font_asset->get_msdf_metrics()->m_font_geometry;
    const auto& metrics = font_geometry.getMetrics();
    const f64 fs_scale = 1 / (metrics.ascenderY - metrics.descenderY);

    f32 cursor_x = 0, cursor_y = 0;
    i32 last_space = -1;
    for (size_t i = 0; i < utf32_text.size(); ++i)
    {
        const auto text_char = utf32_text[i];
        const auto glyph = font_geometry.getGlyph(text_char);
        if (!glyph)
            continue;

        if (text_char != ' ')
        {
            double pl, pb, pr, pt;
            glyph->getQuadPlaneBounds(pl, pb, pr, pt);
            glm::vec2 quad_min{ static_cast<f32>(pl), static_cast<f32>(pb) };
            glm::vec2 quad_max{ static_cast<f32>(pr), static_cast<f32>(pt) };

            quad_min *= fs_scale;
            quad_max *= fs_scale;
            quad_min += glm::vec2{ cursor_x, cursor_y };
            quad_max += glm::vec2{ cursor_x, cursor_y };

            if (quad_max.x > p_max_width && last_space != -1)
            {
                i = last_space;
                if (p_next_lines)
                    p_next_lines->emplace_back(last_space);
                last_space = -1;
                cursor_x = 0;
                cursor_y += fs_scale * metrics.lineHeight + p_line_height_offset;
            }
        }
        else
            last_space = i;

        f64 advance = glyph->getAdvance();
        font_geometry.getAdvance(advance, text_char, utf32_text[i + 1]);
        cursor_x += fs_scale * advance + p_kerning_offset;

        const f32 char_height = glm::trunc(glyph_data.m_size.y * scale);

        width += glm::trunc(static_cast<f32>(glyph_data.m_advance) * scale);

        if (i == 0)
        {
            x += glm::trunc(static_cast<f32>(glyph_data.m_x_off) * scale);
            y -= glm::trunc(static_cast<f32>(glyph_data.m_y_off) * scale);
        }
        else if (i == utf32_text.size() - 1)
        {
            z += width;
            w += glm::trunc(-static_cast<f32>(glyph_data.m_y_off) * scale + char_height);
        }
    }

    return bounding_box{ x, y, z, w };
}

} // end namespace kb::render
