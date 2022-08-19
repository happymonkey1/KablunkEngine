#pragma once

#ifndef KABLUNK_RENDERER_TEXTURE_ATLAS_H
#define KABLUNK_RENDERER_TEXTURE_ATLAS_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Uuid64.h"

#include <glm/glm.hpp>

#include <string>
#include <unordered_map>

namespace Kablunk
{
	class Texture2D;
	class TextureAtlas;

	struct TextureAtlasSprite
	{
		glm::vec2 texture_coords[4];
		// #TODO this should be assetID
		IntrusiveRef<TextureAtlas> texture_atlas;
	};

	class TextureAtlas : public RefCounted
	{
	public:
		TextureAtlas(const std::string& filepath, u32 width = 128ul, u32 height = 128ul);
		TextureAtlas(const std::vector<IntrusiveRef<Texture2D>> textures, u32 width = 128ul, u32 height = 128ul);
		TextureAtlas(u32 width, u32 height);
		~TextureAtlas();

		// get padding value for the texture atlas
		u32 get_padding() const { return m_padding; }
		// set padding value for the texture atlas
		void set_padding(u32 padding) { m_padding = padding; }

		// get width of individual texture in the atlas
		u32 get_texture_width() const { return m_texture_width; }
		// set width of individual texture in the atlas
		void set_texture_width(u32 width) { m_texture_width = width; }

		// get height of individual texture in the atlas
		u32 get_texture_height() const { return m_texture_height; }
		// set height of individual texture in the atlas
		void set_texture_height(u32 height) { m_texture_height = height; }

		// get sprite by uuid
		TextureAtlasSprite get_texture_by_uuid(uuid::uuid64 uuid) const;

		// check if uuid is inside sprite map
		bool is_uuid_in_sprite_map(uuid::uuid64 uuid) const { return m_sprite_map.find(uuid) != m_sprite_map.end(); }

		// get filepath for texture atlas
		const std::string& get_texture_atlas_filepath() const { return m_atlas_filepath; }

		// get texture atlas
		IntrusiveRef<Texture2D> get_texture_atlas() const { return m_atlas_texture; }

		// invalidate texture atlas and free memory
		void invalidate();
	private:
		// split up texture atlas into sub sprites
		void splice_texture_atlas();
	private:
		// primary atlas texture that is used to create sub sprites
		IntrusiveRef<Texture2D> m_atlas_texture;
		// width of textures inside atlas
		u32 m_texture_width = 128ul;
		// height of textures inside atlas
		u32 m_texture_height = 128ul;
		// padding between textures inside atlas
		u32 m_padding = 0ul;
		// map for parsed textures
		std::unordered_map<uuid::uuid64, TextureAtlasSprite> m_sprite_map;
		// filepath of the atlas
		std::string m_atlas_filepath;
	};

}

#endif
