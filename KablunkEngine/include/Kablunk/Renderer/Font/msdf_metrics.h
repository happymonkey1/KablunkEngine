#pragma once

#include <msdf-atlas-gen.h>

namespace kb::render
{ // start namespace kb::render

struct msdf_metrics
{
    msdf_atlas::FontGeometry m_font_geometry{};
    std::vector<msdf_atlas::GlyphGeometry> m_glyphs{};
};

} // end namespace kb::render
