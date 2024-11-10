#include "kablunkpch.h"

#include "Kablunk/Asset/AssetSerializer.h"
#include "Kablunk/Asset/AssetManager.h"

#include "Kablunk/Core/Singleton.h"
#include "Kablunk/Core/Application.h"


#include "Kablunk/Renderer/backend/texture.h"
#include "Kablunk/Audio/AudioAsset.h"
#include "Kablunk/lua/lua_asset.h"

#include "Kablunk/Asset/AssetManager.h"

namespace kb::asset
{
// ======================
// TextureAssetSerializer
// ======================

bool TextureAssetSerializer::try_load_data(const AssetMetadata& metadata, arc<IAsset>& asset) const
{
	asset = render::backend::texture_2d::create(m_asset_manager->get_absolute_path(metadata).string()).As<IAsset>();
	asset->set_id(metadata.id);

    const bool success = asset.As<render::backend::texture_2d>()->loaded();
	if (!success)
		asset->set_flag(AssetFlag::Invalid, true);

	return success;
}

// ======================


// ====================
// AudioAssetSerializer
// ====================


void AudioAssetSerializer::serialize(const AssetMetadata& metadata, arc<IAsset>& asset) const
{
    KB_CORE_WARN("[AudioAssetSerializer]: serialize() not implemented!");
}

bool AudioAssetSerializer::try_load_data(const AssetMetadata& metadata, arc<IAsset>& asset) const
{
    const audio::audio_asset_config config{ m_asset_manager->get_absolute_path(metadata).string() };

	asset = audio::AudioAsset::create(config).As<IAsset>();
	asset->set_id(metadata.id);

    const bool success = asset.As<audio::AudioAsset>()->loaded();
	if (!success)
		asset->set_flag(AssetFlag::Invalid, true);

	return success;
}

// =====================

// =====================
// lua_asset_serializer
// =====================

void lua_asset_serializer::serialize(const AssetMetadata& metadata, arc<IAsset>& asset) const
{
    KB_CORE_WARN("[lua_asset_serializer]: serialize() not implemented!");
}

bool lua_asset_serializer::try_load_data(const AssetMetadata& metadata, arc<IAsset>& asset) const
{
    KB_CORE_ASSERT(false, "not implemented!");

    return nullptr;
}

}
