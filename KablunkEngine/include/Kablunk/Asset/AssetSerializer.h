#pragma once
#ifndef KABLUNK_CORE_ASSET_ASSET_SERIALIZER_H
#define KABLUNK_CORE_ASSET_ASSET_SERIALIZER_H

#include "Kablunk/Asset/Asset.h"
#include "Kablunk/Asset/AssetMetadata.h"
#include "Kablunk/Core/RefCounting.h"

#include <unordered_map>

namespace Kablunk::asset
{ // start namespace Kablunk::asset

// abstract asset serializer base class
class AssetSerializer : public RefCounted
{
public:
    AssetSerializer() = default;
    virtual ~AssetSerializer() = default;

    virtual void serialize(const AssetMetadata& metadata, IntrusiveRef<IAsset>& asset) const = 0;
    virtual bool try_load_data(const AssetMetadata& metadata, IntrusiveRef<IAsset>& asset) const = 0;
};

// text asset
class TextureAssetSerializer : public AssetSerializer
{
public:
    virtual void serialize(const AssetMetadata& metadata, IntrusiveRef<IAsset>& asset) const override {}
    virtual bool try_load_data(const AssetMetadata& metadata, IntrusiveRef<IAsset>& asset) const override;
};

// audio asset
class AudioAssetSerializer : public AssetSerializer
{
public:
    virtual void serialize(const AssetMetadata& metadata, IntrusiveRef<IAsset>& asset) const override;
    virtual bool try_load_data(const AssetMetadata& metadata, IntrusiveRef<IAsset>& asset) const override;
};

// font serializer
class font_asset_serializer : public AssetSerializer
{
public:
    virtual void serialize(const AssetMetadata& metadata, IntrusiveRef<IAsset>& asset) const override;
    virtual bool try_load_data(const AssetMetadata& metadata, IntrusiveRef<IAsset>& asset) const override;
};

} // end namespace Kablunk::asset

#endif
