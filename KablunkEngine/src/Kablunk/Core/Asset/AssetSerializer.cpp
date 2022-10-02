#include "kablunkpch.h"

#include "Kablunk/Asset/AssetSerializer.h"
#include "Kablunk/Asset/AssetManager.h"

#include "Kablunk/Core/Singleton.h"

#include "Kablunk/Renderer/Texture.h"

namespace Kablunk::asset
{
	// ======================
	// TextureAssetSerializer
	// ======================

	bool TextureAssetSerializer::try_load_data(const AssetMetadata& metadata, IntrusiveRef<IAsset>& asset) const
	{
		asset = Texture2D::Create(Singleton<AssetManager>::get().get_absolute_path(metadata).string());
		asset->set_id(metadata.id);

		bool success = asset.As<Texture2D>()->loaded();
		if (!success)
			asset->set_flag(AssetFlag::Invalid, true);

		return success;
	}

	// ======================
}
