#include "kablunkpch.h"
#include "Kablunk/Renderer/Font/FontManager.h"

#include "Kablunk/Asset/AssetCommand.h"

namespace kb::render
{ // start namespace kb::render

void font_manager::init()
{
	KB_CORE_ASSERT(!m_ft_library, "[font_manager]: freetype rendering engine already initialized!");

	// initialize freetype
	auto error = FT_Init_FreeType(&m_ft_library);
	if (error != FT_Err_Ok)
		KB_CORE_ASSERT(false, "[font_manager]: failed to initialize freetype! error code '{}'", error);
}

void font_manager::shutdown()
{
	if (m_ft_library)
		FT_Done_FreeType(m_ft_library);
}

void font_manager::add_font_file_to_library(
    ref<font_asset_t> font_asset,
    const std::filesystem::path& p_absolute_filepath
)
{
	KB_CORE_ASSERT(font_asset->is_valid(), "[font_manager]: font asset is invalid!");

	// check whether the font cache is already in the map
	auto it = m_font_cache.find(font_asset->get_id());
	if (it != m_font_cache.end())
	{
		KB_CORE_WARN("[font_manager]: tried adding a font asset '{}' but it already exists in the cache!", font_asset->get_id());
		return;
	}

	// check whether we need to try loading the font file into memory
	// loading the ft face will be skipped if set to false
	const bool load_memory = font_asset->is_flag_set(asset::asset_flag_t::Invalid);

	const font_asset_create_info_t create_info{
        p_absolute_filepath.string(),		// absolute path to the font file
		font_asset->get_font_point(),						// font point 
		m_ft_library,				// pointer to the font engine
		0ull,						// face index to load
        128ull,                     // number of glyphs to load
		load_memory					// whether to load the font into memory
	};

	// try to load font asset into memory if not already loaded
	// set invalid bit on fail
	if (!font_asset->is_valid() && !font_asset->load_ft_face_from_file(create_info))
	{
		font_asset->set_flag(asset::asset_flag_t::Invalid);
		return;
	}

	// check for programmer error
	KB_CORE_ASSERT(!font_asset->is_flag_set(asset::asset_flag_t::Invalid) && font_asset->m_ft_face, "Invalid bit not set and face not loaded???");

	// add font to cache
	m_font_cache.emplace(font_asset->get_id(), font_asset);

    KB_CORE_INFO("[font_manager]: added '{}' to the font asset registry", p_absolute_filepath.string());
}

void font_manager::remove_font_file_from_library(ref<font_asset_t> font_asset)
{
	KB_CORE_ASSERT(false, "not implemented!");
}

ref<kb::render::font_asset_t> font_manager::get_font_asset(asset::asset_id_t p_font_asset_id) const
{
	return m_font_cache.contains(p_font_asset_id) ? m_font_cache.at(p_font_asset_id) : ref<font_asset>{};
}

bool font_manager::has_font_cached(ref<font_asset_t> font_asset)
{
	KB_CORE_ASSERT(font_asset, "null font asset?");

	return m_font_cache.contains(font_asset->get_id());
}

} // end namespace kb::render
