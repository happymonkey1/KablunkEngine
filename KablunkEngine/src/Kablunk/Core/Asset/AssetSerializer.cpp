#include "kablunkpch.h"

#include "Kablunk/Asset/AssetSerializer.h"
#include "Kablunk/Asset/AssetManager.h"

#include "Kablunk/Core/Singleton.h"
#include "Kablunk/Core/Application.h"

#include "Kablunk/Renderer/RenderCommand2D.h"

#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Audio/AudioAsset.h"
#include "Kablunk/Renderer/Font/FontAsset.h"
#include "Kablunk/lua/lua_asset.h"

#include "Kablunk/Asset/AssetManager.h"

namespace kb::asset
{
// ======================
// TextureAssetSerializer
// ======================

bool TextureAssetSerializer::try_load_data(const AssetMetadata& metadata, ref<IAsset>& asset) const
{
	asset = Texture2D::Create(m_asset_manager->get_absolute_path(metadata).string());
	asset->set_id(metadata.id);

	bool success = asset.As<Texture2D>()->loaded();
	if (!success)
		asset->set_flag(AssetFlag::Invalid, true);

	return success;
}

// ======================


// ====================
// AudioAssetSerializer
// ====================


void AudioAssetSerializer::serialize(const AssetMetadata& metadata, ref<IAsset>& asset) const
{
    KB_CORE_WARN("[AudioAssetSerializer]: serialize() not implemented!");
}

bool AudioAssetSerializer::try_load_data(const AssetMetadata& metadata, ref<IAsset>& asset) const
{
	audio::audio_asset_config config{ m_asset_manager->get_absolute_path(metadata).string() };

	asset = audio::AudioAsset::create(config);
	asset->set_id(metadata.id);

	bool success = asset.As<audio::AudioAsset>()->loaded();
	if (!success)
		asset->set_flag(AssetFlag::Invalid, true);

	return success;
}

// ====================

// =====================
// font_asset_serializer
// =====================

void font_asset_serializer::serialize(const AssetMetadata& metadata, ref<IAsset>& asset) const
{
    KB_CORE_WARN("[font_asset_serializer]: serialize() not implemented!");
}

bool font_asset_serializer::try_load_data(const AssetMetadata& metadata, ref<IAsset>& asset) const
{
    const render::font_asset_create_info font_create_info{
        // path to font asset
        m_asset_manager->get_absolute_path(metadata).string(),
        // font point
        render::font_manager::k_load_font_point,
        // #TODO get rid of singleton reference
        // underlying font engine
        Application::Get().get_renderer_2d()->get_font_manager().get_ft_engine(),
        // font face index to load
        0ull,
        // number of glyphs to load from the font
        // #TODO this is probably wrong for non-ascii fonts
        128ull,
        // load font into memory
        true
    };

    asset = render::font_asset_t::create(font_create_info);
    if (asset)
    {
        ref<render::font_asset_t> font_asset = asset.As<render::font_asset_t>();
        font_asset->set_id(metadata.id);
    }
    else
        asset->set_flag(AssetFlag::Invalid, true);

    return asset;
}

// =====================

// =====================
// lua_asset_serializer
// =====================

void lua_asset_serializer::serialize(const AssetMetadata& metadata, ref<IAsset>& asset) const
{
    KB_CORE_WARN("[lua_asset_serializer]: serialize() not implemented!");
}

bool lua_asset_serializer::try_load_data(const AssetMetadata& metadata, ref<IAsset>& asset) const
{
    KB_CORE_ASSERT(false, "not implemented!");

    return nullptr;
}

}
