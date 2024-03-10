#pragma once
#include "Kablunk/Asset/Asset.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/Font/msdf_metrics.h"

#include <filesystem>

namespace kb::render
{ // start namespace kb::render

class font : public asset::Asset
{
public:
    font() noexcept = default;
    explicit font(const std::filesystem::path& p_path) noexcept;
    explicit font(std::string p_name, owning_buffer&& p_buffer) noexcept;
    ~font() noexcept override = default;

    // get a mutable reference to the underlying texture atlas
    auto get_font_atlas() const noexcept -> ref<Texture2D> { return m_texture_atlas; }
    // get an immutable pointer to the glyph metrics
    auto get_msdf_metrics() const noexcept -> const msdf_metrics* { KB_CORE_ASSERT(m_msdf_metrics, "[font]: msdf metrics is null?"); return m_msdf_metrics.get(); }

    static auto get_static_type() -> asset::AssetType { return asset::AssetType::Font; }

    // get a string_view to the font's name
    auto get_name() const noexcept -> std::string_view { return std::string_view{ m_name }; }
private:
    auto generate_atlas(owning_buffer&& p_font_data_buffer) noexcept -> void;
private:
    // debug name
    std::string m_name{};
    // underlying texture
    ref<Texture2D> m_texture_atlas{};
    // glyph metrics
    std::unique_ptr<msdf_metrics> m_msdf_metrics{};
};

} // end namespace kb::render
