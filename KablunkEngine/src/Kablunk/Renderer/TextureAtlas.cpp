#include "kablunkpch.h"

#include "Kablunk/Renderer/TextureAtlas.h"
#include "Kablunk/Renderer/Texture.h"

namespace Kablunk
{

	TextureAtlas::TextureAtlas(const std::string& filepath, u32 width /*= 128ul*/, u32 height /*= 128ul*/)
		: m_atlas_filepath{ filepath }, m_texture_width{ width }, m_texture_height{ height }
	{
		m_atlas_texture = Texture2D::Create(m_atlas_filepath);

		KB_CORE_ASSERT(m_atlas_texture->GetFormat() == ImageFormat::RGBA, "only RGBA image format is supported!");

		splice_texture_atlas();
	}

	TextureAtlas::TextureAtlas(u32 width, u32 height)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	TextureAtlas::TextureAtlas(const std::vector<IntrusiveRef<Texture2D>> textures, u32 width /*= 128ul*/, u32 height /*= 128ul*/)
	{
		KB_CORE_ASSERT(false, "not implemented");
	}

	TextureAtlas::~TextureAtlas()
	{
		invalidate();
	}

	TextureAtlasSprite TextureAtlas::get_texture_by_uuid(uuid::uuid64 uuid) const
	{
		KB_CORE_ASSERT(m_sprite_map.find(uuid) != m_sprite_map.end(), "uuid not found in sprite map!");

		return m_sprite_map.at(uuid);
	}

	void TextureAtlas::invalidate()
	{
		m_atlas_texture->GetImage()->Invalidate();

		m_sprite_map.clear();
	}

	void TextureAtlas::splice_texture_atlas()
	{
		// #TODO should the entire map be cleared?
		if (!m_sprite_map.empty())
			m_sprite_map.clear();
		
		KB_CORE_ASSERT(m_padding == 0ul, "only padding == 0 is supported!");

		u32 width = m_atlas_texture->GetWidth(), height = m_atlas_texture->GetHeight();
		size_t columns = width / m_texture_width;
		size_t rows = height / m_texture_height;

		size_t max_sprite_count = rows * columns;
		m_sprite_map.reserve(max_sprite_count);

		constexpr float border_uv_offset_x = 0.05f;
		constexpr float border_uv_offset_y = 0.05f;

		float texture_uv_width = static_cast<float>(m_texture_width) / static_cast<float>(width);
		float texture_uv_height = static_cast<float>(texture_uv_height) / static_cast<float>(height);


		for (size_t i = 0; i < max_sprite_count; ++i)
		{
			TextureAtlasSprite& sprite = m_sprite_map.emplace(uuid::generate(), TextureAtlasSprite{}).first->second;
			
			float y = i / columns;
			float x = i % columns;
			
			// calculate uv values
			sprite.texture_coords[0] = glm::vec2{
				(float)(border_uv_offset_x + x * m_texture_width) / width,
				(float)(border_uv_offset_y + y * m_texture_height) / height
			};
			sprite.texture_coords[1] = glm::vec2{ 
				(float)(-border_uv_offset_x + x * m_texture_width) / width + texture_uv_width,
				(float)(border_uv_offset_y + y * m_texture_height) / height
			};
			sprite.texture_coords[2] = glm::vec2{
				(float)(-border_uv_offset_x + x * m_texture_width) / width + texture_uv_width,
				(float)(-border_uv_offset_y + y * m_texture_height) / height + texture_uv_height
			};
			sprite.texture_coords[3] = glm::vec2{
				(float)(border_uv_offset_x + x * m_texture_width) / width,
				(float)(-border_uv_offset_y + y * m_texture_height) / height + texture_uv_height
			};
			sprite.texture_atlas = m_atlas_texture;
		}
#if 0
		// #TODO I think this only works for RGBA textures (not RGBA16, RGBA32)

		u32 width = m_atlas_texture->GetWidth(), height = m_atlas_texture->GetHeight();

		size_t max_sprite_count = (width / m_texture_width) * (height / m_texture_height);
		m_sprite_map.reserve(max_sprite_count);

		const Buffer& atlas_buffer = m_atlas_texture->GetImage()->GetBuffer();
		Buffer* sprites = new Buffer[max_sprite_count];
		for (size_t y = 0; y < height; ++y)
		{
			for (size_t x = 0; x < width; ++x)
			{
				// calculate index of pixel in atlas data buffer
				size_t index = y * width + x;
				// calculate index of sprite
				size_t sprite_index = ((y * width) % m_texture_height) + (x % m_texture_width);
				// calculate index of pixel in sprite's data buffer
				size_t sprite_pixel_index = ((y * width) / m_texture_height) + (x / m_texture_width);

				// copy pixel data from atlas buffer to sprite image buffer
				sprites[sprite_index][sprite_pixel_index] = atlas_buffer[index];
			}
		}

		// check if created sprite is fully transparent
		for (size_t i = 0; i < max_sprite_count; ++i)
		{
			const Buffer& image_buffer = sprites[i];
			bool valid = false;

			// search through buffer and check for pixel that is not fully transparent.
			// texture is valid if found
			for (size_t j = 0; j < image_buffer.size(); ++j)
				if ((image_buffer[j] | 0xFF) > 0)
					valid = true;

			if (valid)
			{
				// assumes texture2D copies passed in buffer data
				// #TODO data should not be copied, since we can reference the texture atlas buffer
				m_sprite_map.emplace(
					uuid::generate(), 
					Texture2D::Create(m_atlas_texture->GetFormat(), m_texture_width, m_texture_height, image_buffer.get())
				);
			}
		}

		delete[] sprites;
#endif
	}

}
