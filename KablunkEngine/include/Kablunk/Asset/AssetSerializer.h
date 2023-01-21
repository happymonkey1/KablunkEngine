#pragma once
#ifndef KABLUNK_CORE_ASSET_ASSET_SERIALIZER_H
#define KABLUNK_CORE_ASSET_ASSET_SERIALIZER_H

#include "Kablunk/Asset/Asset.h"
#include "Kablunk/Asset/AssetMetadata.h"
#include "Kablunk/Core/RefCounting.h"

#include <unordered_map>

namespace Kablunk::asset
{

	class AssetSerializer : public RefCounted
	{
	public:
		AssetSerializer() = default;
		virtual ~AssetSerializer() = default;

		virtual void serialize(const AssetMetadata& metadata, IntrusiveRef<IAsset>& asset) const = 0;
		virtual bool try_load_data(const AssetMetadata& metadata, IntrusiveRef<IAsset>&  asset) const = 0;
	};

	class TextureAssetSerializer : public AssetSerializer
	{
	public:
		virtual void serialize(const AssetMetadata& metadata, IntrusiveRef<IAsset>& asset) const override {}
		virtual bool try_load_data(const AssetMetadata& metadata, IntrusiveRef<IAsset>& asset) const override;
	};

	class AudioAssetSerializer : public AssetSerializer
	{
	public:
		virtual void serialize(const AssetMetadata& metadata, IntrusiveRef<IAsset>& asset) const override;
		virtual bool try_load_data(const AssetMetadata& metadata, IntrusiveRef<IAsset>& asset) const override;
	};
}

#endif
