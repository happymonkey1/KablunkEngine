#pragma once

#ifndef KABLUNK_RENDERER_TEXTURE_ATLAS_H
#define KABLUNK_RENDERER_TEXTURE_ATLAS_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Uuid64.h"

#include <glm/glm.hpp>

#include <string>
#include <unordered_map>

// forward declaration
namespace kb
{
    class Texture2D;
}

namespace kb::render
{

class texture_atlas;

struct TextureAtlasSprite
{
	glm::vec2 texture_coords[4];
	// #TODO this should be assetID
	kb::ref<texture_atlas> texture_atlas;
};

class texture_atlas : public kb::RefCounted
{
public:
    texture_atlas(const std::string& filepath, u32 width = 128ul, u32 height = 128ul);
    texture_atlas(const std::vector<ref<kb::Texture2D>> textures, u32 width = 128ul, u32 height = 128ul);
    texture_atlas(u32 width, u32 height);
    ~texture_atlas();
    
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
    TextureAtlasSprite get_texture_by_uuid(kb::uuid::uuid64 uuid) const;
    
    // check if uuid is inside sprite map
    bool is_uuid_in_sprite_map(kb::uuid::uuid64 uuid) const { return m_sprite_map.find(uuid) != m_sprite_map.end(); }
    
    // get filepath for texture atlas
    const std::string& get_texture_atlas_filepath() const { return m_atlas_filepath; }
    
    // get a mutable reference to the texture atlas
    ref<kb::Texture2D> get_texture_atlas() { return m_texture_atlas; }
    // get an immutable reference to the texture atlas
    const ref<kb::Texture2D>& get_texture_atlas() const { return m_texture_atlas; }
    
    // invalidate texture atlas and free memory
    void invalidate();
private:
    // split up texture atlas into sub sprites
    void splice_texture_atlas();
    
    // create a texture atlas from individual textures
    void create_texture_atlas_from_images(const std::vector<ref<kb::Texture2D>> textures);
private:
    // primary atlas texture that is used to create sub sprites
    ref<kb::Texture2D> m_texture_atlas;
    // width of textures inside atlas
    u32 m_texture_width = 0ul;
    // height of textures inside atlas
    u32 m_texture_height = 0ul;
    // padding between textures inside atlas
    u32 m_padding = 0ul;
    // map for parsed textures
    std::unordered_map<kb::uuid::uuid64, TextureAtlasSprite> m_sprite_map;
    // filepath of the atlas
    std::string m_atlas_filepath;
};

}

#endif
