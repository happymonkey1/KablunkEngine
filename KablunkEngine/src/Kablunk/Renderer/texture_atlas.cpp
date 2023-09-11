#include "kablunkpch.h"

#include "Kablunk/Renderer/texture_atlas.h"
#include "Kablunk/Renderer/Texture.h"

namespace kb::render
{ // start namespace kb::render

texture_atlas::texture_atlas(const std::string& filepath, u32 width /*= 128ul*/, u32 height /*= 128ul*/)
	: m_atlas_filepath{ filepath }, m_texture_width{ width }, m_texture_height{ height }
{
	m_texture_atlas = kb::Texture2D::Create(m_atlas_filepath);

	KB_CORE_ASSERT(m_texture_atlas->GetFormat() == kb::ImageFormat::RGBA, "only RGBA image format is supported!");

	splice_texture_atlas();
}

texture_atlas::texture_atlas(u32 width, u32 height)
	: m_texture_width{ width }, m_texture_height{ height }
{
	KB_CORE_ASSERT(false, "not implemented!");
}

texture_atlas::texture_atlas(const std::vector<ref<kb::Texture2D>> textures, u32 width /*= 128ul*/, u32 height /*= 128ul*/)
	: m_texture_width{ width }, m_texture_height{ height }
{
	KB_CORE_ASSERT(textures.size() > 0, "there are no textures in the input textures list");
	KB_CORE_ASSERT(textures[0]->GetFormat() == kb::ImageFormat::RGBA, "only RGBA image format is supported!");
		
	create_texture_atlas_from_images(textures);
	splice_texture_atlas();
}

texture_atlas::~texture_atlas()
{
	invalidate();
}

TextureAtlasSprite texture_atlas::get_texture_by_uuid(kb::uuid::uuid64 uuid) const
{
	KB_CORE_ASSERT(m_sprite_map.find(uuid) != m_sprite_map.end(), "uuid not found in sprite map!");

	return m_sprite_map.at(uuid);
}

void texture_atlas::invalidate()
{
	m_texture_atlas->GetImage()->Invalidate();

	m_sprite_map.clear();
}

void texture_atlas::splice_texture_atlas()
{
	// #TODO should the entire map be cleared?
	if (!m_sprite_map.empty())
		m_sprite_map.clear();
		
	KB_CORE_ASSERT(m_padding == 0ul, "only padding == 0 is supported!");

	u32 width = m_texture_atlas->GetWidth(), height = m_texture_atlas->GetHeight();
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
		TextureAtlasSprite& sprite = m_sprite_map.emplace(kb::uuid::generate(), TextureAtlasSprite{}).first->second;
			
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
		sprite.texture_atlas = this;
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

void texture_atlas::create_texture_atlas_from_images(const std::vector<ref<kb::Texture2D>> textures)
{
	size_t sprite_count = textures.size();

	// #TODO allow textures with different dimensions
	for (const auto& texture : textures)
		KB_CORE_ASSERT(texture->GetWidth() == textures[0]->GetWidth() && texture->GetHeight() == textures[0]->GetHeight(), "Texture Atlas only supports textures with the same dimensions!");

	// choose the best atlas size
	constexpr const size_t atlas_sizes[4] = { 1024ull, 2048ull, 4096ull, 8192ull };
	std::array<size_t, 4> unused_sprite_counts{};
	for (size_t i = 0; i < 4; ++i)
	{
		const size_t width = atlas_sizes[i];
		const size_t sprites_per_row = width / m_texture_width;
		i32 unused_count = static_cast<i32>(sprites_per_row * sprites_per_row) - static_cast<i32>(sprites_per_row);
		if (unused_count < 0)
			unused_count = static_cast<i32>(width + 1);

		unused_sprite_counts[i] = static_cast<size_t>(unused_count);
	}

	// get final atlas sized based off the one that minimizes unused space
	const size_t final_atlas_width = atlas_sizes[std::min_element(unused_sprite_counts.begin(), unused_sprite_counts.end()) - unused_sprite_counts.begin()];
		
	// #TODO expose other image specifications
    kb::ImageSpecification image_spec{};
	image_spec.width  = static_cast<uint32_t>(final_atlas_width);
	image_spec.height = static_cast<uint32_t>(final_atlas_width);

	KB_CORE_ASSERT(textures[0]->GetImage()->GetSpecification().layers == 4, "assertion that RGBA has 4 layers");
    kb::Buffer atlas_image_data_buffer{ final_atlas_width * final_atlas_width * textures[0]->GetImage()->GetSpecification().layers };

	size_t x = 0, y = 0;
	// #TODO out of bounds checks
	for (const auto& texture : textures)
	{
		size_t width = static_cast<size_t>(texture->GetWidth());
		size_t height = static_cast<size_t>(texture->GetHeight());
		for (size_t texture_y = 0; texture_y < height; ++texture_y)
		{
			for (size_t texture_x = 0; texture_x < width; ++texture_x)
			{
				// calculate index of pixel in atlas data buffer
				size_t atlas_buffer_index = y * final_atlas_width + x;

				// calculate index of sprite
				size_t texture_index = texture_y * final_atlas_width + texture_x;

				atlas_image_data_buffer[atlas_buffer_index] = texture->GetImage()->GetBuffer()[texture_index];

				if (++x >= final_atlas_width)
				{
					x = 0;
					y++;
					KB_CORE_ASSERT(y < final_atlas_width, "out of texture bounds!");
				}
			}
		}
	}

	m_texture_atlas = kb::Texture2D::Create(textures[0]->GetFormat(), final_atlas_width, final_atlas_width, atlas_image_data_buffer.get());

	// #TODO save atlas texture to disk so processing is only done once

	atlas_image_data_buffer.Release();
}

}
