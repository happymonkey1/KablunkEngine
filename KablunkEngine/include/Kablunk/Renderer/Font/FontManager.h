#pragma once
#ifndef KABLUNK_RENDERER_FONT_FONT_MANAGER_H
#define KABLUNK_RENDERER_FONT_FONT_MANAGER_H

#include "Kablunk/Renderer/Font/FontAsset.h"
#include "Kablunk/Core/Singleton.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <unordered_map>

namespace Kablunk::render
{

	class font_manager
	{
	public:
		// initialization for the font manager
		// initialize the underlying font rendering engine (freetype)
		void init();
		// shutdown for the font manager
		void shutdown();
		// try to add a font to the font cache
		void add_font_file_to_library(ref<font_asset_t> font_asset);
		// try to remove a font from the font cache
		void remove_font_file_from_library(ref<font_asset_t> font_asset);
		// get a reference to the font engine
		FT_Library& get_ft_engine() { return m_ft_library; }
		// get a reference to the font engine
		const FT_Library& get_ft_engine() const { return m_ft_library; }
		// get the font cache
		// maps relative font paths to font assets
		const std::unordered_map<std::string, ref<font_asset_t>> get_font_cache() const { return m_font_cache; }
	private:
		// cache of fonts loaded into memory
		// maps relative font paths to font assets
		std::unordered_map<std::string, ref<font_asset_t>> m_font_cache;
		// underlying font rendering engine (freetype)
		FT_Library m_ft_library = nullptr;
	};

}

#endif
