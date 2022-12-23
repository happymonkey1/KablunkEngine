#include "kablunkpch.h"
#include "Kablunk/Renderer/Font/FontAsset.h"

namespace Kablunk::render
{

	font_asset::font_asset(const font_asset_create_info_t& create_info)
		: m_font_point{ create_info.m_font_point }, m_font_face_index{ create_info.m_font_face_index }, m_num_glyphs{ create_info.m_num_glyphs }
	{

	}

	font_asset::~font_asset()
	{
		release();
	}

	ref<font_asset> font_asset::create(const font_asset_create_info_t& create_info)
	{
		ref<font_asset_t> font_asset = ref<font_asset_t>::Create(create_info);

		// try load font into memory
		if (create_info.m_load_memory && font_asset->load_ft_face_from_file(create_info))
			return font_asset;
		
		// if we did not load into memory or failed to, set invalid flag
		font_asset->set_flag(asset::asset_flag_t::Invalid);
		return font_asset;
	}

	void font_asset::set_font_point(size_t new_font_point)
	{
		m_font_point = new_font_point;
		auto error = FT_Set_Char_Size(
			m_ft_face,			/* handle to face object         */
			0,					/* char_width in 1/64 of points  */
			m_font_point * 64,	/* char_height in 1/64 of points */
			0,					/* horizontal device resolution  */
			0					/* vertical device resolution    */
		);   

		if (error)
			KB_CORE_ASSERT(false, "[font_asset]: failed to change font point! error code '{}'", error)
	}

	void font_asset::release()
	{
		// #NOTE(Sean) should this release/invalidate the texture instead of just releasing the reference?
		if (m_texture_atlas)
			m_texture_atlas = nullptr;

		// #TODO release ft font face
		if (m_ft_face)
		{
			FT_Done_Face(m_ft_face);
			m_ft_face = nullptr;
		}

		m_glyph_info_map.clear();
	}

	bool font_asset::load_ft_face_from_file(const font_asset_create_info_t& create_info)
	{
		if (m_ft_face)
		{
			KB_CORE_WARN("[font_asset]: font asset '{}' already has a font face loaded!", get_id());
			// NOTE(Sean) should we return false?
			return true;
		}

		// try loading font face using FT_New_Face
		auto error = FT_New_Face(
			create_info.m_ft_engine,
			create_info.m_absolute_filepath.c_str(),
			create_info.m_font_face_index,
			&m_ft_face
		);

		if (error == FT_Err_Unknown_File_Format)
		{
			KB_CORE_ERROR(
				"[font_asset]: the font file '{}' could be opened and read, but it seems like an unsupported type!",
				create_info.m_absolute_filepath
			);

			return false;
		}
		else if (error)
		{
			KB_CORE_ERROR(
				"[font_asset]: the font file '{}' could not opened, read, or the file is broken!",
				create_info.m_absolute_filepath
			);

			return false;
		}

		if (!m_texture_atlas)
			create_texture_atlas();

		return true;
	}

	void font_asset::create_texture_atlas()
	{
		// check if we have already rendered out a texture atlas
		if (!m_glyph_info_map.empty())
		{
			KB_CORE_WARN("[font_asset]: tried to create a texture map, but glyph info already exists!");
			return;
		}

		if (m_texture_atlas)
		{
			KB_CORE_WARN("[font_asset]: tried to create a texture map, but texture data is already cached!");
			return;
		}

		m_glyph_info_map.reserve(m_num_glyphs);

		// quick and dirty max texture size estimate
		const u32 max_dim = (1 + (m_ft_face->size->metrics.height >> 6)) * std::ceilf(std::sqrtf(static_cast<f32>(m_num_glyphs)));
		u32 tex_width = 1;
		while (tex_width < max_dim) 
			tex_width <<= 1;
		const u32 tex_height = tex_width;

		// create "texture atlas" in memory
		// #TODO use kablunk texture atlas
		u8* pixel_data = new u8[tex_width * tex_height]{ 1 };
		size_t pen_x = 0;
		size_t pen_y = 0;

		// load glyph data and store info in map
		for (size_t i = 0; i < m_num_glyphs; ++i) {
			FT_Load_Char(m_ft_face, i, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
			FT_Bitmap* bmp = &m_ft_face->glyph->bitmap;

			if (pen_x + bmp->width >= tex_width) {
				pen_x = 0;
				pen_y += ((m_ft_face->size->metrics.height >> 6) + 1);
			}

			for (size_t row = 0; row < bmp->rows; ++row) {
				for (size_t col = 0; col < bmp->width; ++col) {
					const size_t x = pen_x + col;
					const size_t y = pen_y + row;

					const size_t pixel_index = y * tex_width + x;
					const size_t buffer_index = row * bmp->pitch + col;
					KB_CORE_ASSERT(pixel_index <= tex_width * tex_height, "pixel buffer overflow");
					KB_CORE_ASSERT(buffer_index <= bmp->width * bmp->rows, "bmp buffer overflow");
					pixel_data[pixel_index] = bmp->buffer[buffer_index];
				}
			}

			// freetype glyph rendering data
			glyph_info_t glyph_data{
				pen_x,
				pen_y,
				pen_x + bmp->width,
				pen_y + bmp->rows,
				m_ft_face->glyph->bitmap_left,
				m_ft_face->glyph->bitmap_top,
				m_ft_face->glyph->advance.x >> 6
			};

			m_glyph_info_map.emplace(static_cast<char>(i), glyph_data);

			pen_x += bmp->width + 1;
		}

		// store pixel data in texture
		m_texture_atlas = Texture2D::Create(ImageFormat::RGBA, tex_width, tex_height, pixel_data);

		delete[] pixel_data;

		return;
	}

}