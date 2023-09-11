#pragma once
#ifndef KABLUNK_RENDERER_FONT_FONT_ASSET_H
#define KABLUNK_RENDERER_FONT_FONT_ASSET_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Asset/Asset.h"

#include "Kablunk/Renderer/Texture.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <unordered_map>

namespace kb::render
{

	struct font_asset_create_info
	{
		// path for the font file
		std::string m_absolute_filepath = "INVALID_FONT_FILEPATH";
		// font point
		size_t m_font_point = 16ull;
		// pointer to the font rendering engine (freetype)
		FT_Library m_ft_engine = nullptr;
		// font face to load from the file
		size_t m_font_face_index = 0ull;
		// number of glyphs to load from the font
		size_t m_num_glyphs = 128ull;
		// whether to initialize and load the font into memory
		bool m_load_memory = true;
	};

	using font_asset_create_info_t = font_asset_create_info;
	
	// info for glyph rendering
	// from https://gist.github.com/baines/b0f9e4be04ba4e6f56cab82eef5008ff
	struct glyph_info
	{
		// start and end positions in the texture atlas
		float m_x0, m_y0, m_x1, m_y1;
		// left & top bearing when rendering
		size_t m_x_off, m_y_off;
		// x advance when rendering
		size_t m_advance;
        // size of the glyph
        glm::vec2 m_size;
	};

	using glyph_info_t = glyph_info;

	class font_asset : public asset::Asset
	{
	public:
		font_asset() = default;
		font_asset(const font_asset_create_info_t& create_info);
		virtual ~font_asset();

		// factory function to create a font_asset instance
		static ref<font_asset> create(const font_asset_create_info_t& create_info);
		// get the font point 
		// font points are a physical distance, representing 1/72th of an inch
		size_t get_font_point() const { return m_font_point; }
        // get the horizontal dpi used to render bitmaps
        size_t get_dpi_x() const { return m_dpi_x; }
        // get the vertical dpi used to render bitmaps
        size_t get_dpi_y() const { return m_dpi_y; }
		// set the font point
		// font points are a physical distance, representing 1/72th of an inch
		void set_font_point(size_t new_font_point);
		// get the texture atlas
		ref<Texture2D> get_texture_atlas() const { return m_texture_atlas; }
		// release resources owned by the asset
		void release();
		// get the glpyh rendering info map
		const std::unordered_map<char, glyph_info_t> get_glyph_rendering_map() const { return m_glyph_info_map; }
	private:
		// load an ft face to the asset
		bool load_ft_face_from_file(const font_asset_create_info_t& create_info);
		// quick and dirty method to create a texture atlas (reference https://gist.github.com/baines/b0f9e4be04ba4e6f56cab82eef5008ff)
		// #TODO update and use the kablunk texture atlas
		void create_texture_atlas();
	private:
		// #TODO support multiple faces
		// freetype font face that is loaded
		FT_Face m_ft_face = nullptr;
		// font point
		size_t m_font_point = 16ull;
		// font face index
		size_t m_font_face_index = 0ull;
		// number of glyphs loaded
		size_t m_num_glyphs = 0ull;
        // horizontal dpi used for rendering the glyph bitmaps
        size_t m_dpi_x = 300ull;
        // vertical dpi used for rendering the glyph bitmaps
        size_t m_dpi_y = 300ull;
		// reference to the texture atlas
		// for now, the font asset "owns" the texture atlas, should this be stored in a cache on the font manager instead?
		ref<Texture2D> m_texture_atlas;
		// glyph rendering info
		// maps chars to their rendering info
		std::unordered_map<char, glyph_info_t> m_glyph_info_map;
		// friend declaration(s)
		friend class font_manager;
	};

	using font_asset_t = font_asset;

}

#endif
