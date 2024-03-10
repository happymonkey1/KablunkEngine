#pragma once
#ifndef KABLUNK_RENDERER_FONT_FONT_MANAGER_H
#define KABLUNK_RENDERER_FONT_FONT_MANAGER_H

#include "Kablunk/Renderer/Font/FontAsset.h"
#include "Kablunk/Core/Singleton.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <unordered_map>

#include "Kablunk/Renderer/Font/font.h"

namespace kb::render
{

class font_manager
{
public:
    // font point at which the glyphs are loaded as
    static constexpr u32 k_load_font_point = 96;
public:
	// initialization for the font manager
	// initialize the underlying font rendering engine (freetype)
    [[deprecated]] void init();
	// shutdown for the font manager
    [[deprecated]] void shutdown();
	// try to add a font to the font cache
    [[deprecated]] void add_font_file_to_library(ref<font_asset_t> font_asset, const std::filesystem::path& p_absolute_filepath);
	// check if a reference to the given font asset is in the cache
    [[deprecated]] bool has_font_cached(ref<font_asset_t> font_asset) const;
	// try to remove a font from the font cache
    [[deprecated]] void remove_font_file_from_library(ref<font_asset_t> font_asset);
	// try get a font by filename
    [[deprecated]] ref<font_asset_t> get_font_asset(asset::asset_id_t p_font_asset_id) const;
	// get a reference to the font engine
    [[deprecated]] FT_Library& get_ft_engine() { return m_ft_library; }
	// get a reference to the font engine
    [[deprecated]] const FT_Library& get_ft_engine() const { return m_ft_library; }
	// get the font cache
	// maps relative font paths to font assets
    [[deprecated]] const kb::unordered_flat_map<asset::asset_id_t, ref<font_asset_t>>& get_font_cache() const { return m_font_cache; }

    // get default font loaded by the engine
    [[nodiscard]] auto get_default_font() const noexcept -> kb::ref<render::font> { return m_default_font; }
    // get the default monospaced font loaded by the engine
    [[nodiscard]] auto get_default_monospaced_font() const noexcept -> kb::ref<render::font> { return m_default_monospaced_font; }
private:
	// cache of fonts loaded into memory
	// maps font filename to font assets
	kb::unordered_flat_map<asset::asset_id_t, ref<font_asset_t>> m_font_cache;
	// underlying font rendering engine (freetype)
	FT_Library m_ft_library = nullptr;
    // default font
    ref<font> m_default_font = ref<font>::Create("resources/fonts/inter/Inter-Regular.ttf");
    // defualt monospaced font
    ref<font> m_default_monospaced_font = ref<font>::Create("resources/fonts/jetbrains-mono/JetBrainsMono-Regular.ttf");
};

}

#endif
