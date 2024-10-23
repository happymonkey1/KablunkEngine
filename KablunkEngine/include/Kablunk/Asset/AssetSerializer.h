#pragma once
#ifndef KABLUNK_CORE_ASSET_ASSET_SERIALIZER_H
#define KABLUNK_CORE_ASSET_ASSET_SERIALIZER_H

#include "Kablunk/Asset/Asset.h"
#include "Kablunk/Asset/AssetMetadata.h"
#include "Kablunk/Core/RefCounting.h"

#include <unordered_map>

namespace kb::asset
{ // start namespace kb::asset

// forward declaration
class AssetManager;

// abstract asset serializer base class
class AssetSerializer : public RefCounted
{
public:
    AssetSerializer() = default;
    AssetSerializer(arc<AssetManager> p_asset_manager) : m_asset_manager{ p_asset_manager } { }
    virtual ~AssetSerializer() = default;

    virtual void serialize(const AssetMetadata& metadata, arc<IAsset>& asset) const = 0;
    virtual bool try_load_data(const AssetMetadata& metadata, arc<IAsset>& asset) const = 0;

protected:
    arc<AssetManager> m_asset_manager;
};

// text asset
class TextureAssetSerializer : public AssetSerializer
{
public:
    TextureAssetSerializer(arc<AssetManager> p_asset_manager) : AssetSerializer{ p_asset_manager } {}

    virtual void serialize(const AssetMetadata& metadata, arc<IAsset>& asset) const override {}
    virtual bool try_load_data(const AssetMetadata& metadata, arc<IAsset>& asset) const override;
};

// audio asset
class AudioAssetSerializer : public AssetSerializer
{
public:
    AudioAssetSerializer(arc<AssetManager> p_asset_manager) : AssetSerializer{ p_asset_manager } {}

    virtual void serialize(const AssetMetadata& metadata, arc<IAsset>& asset) const override;
    virtual bool try_load_data(const AssetMetadata& metadata, arc<IAsset>& asset) const override;
};

// font serializer
class font_asset_serializer : public AssetSerializer
{
public:
    font_asset_serializer(arc<AssetManager> p_asset_manager) : AssetSerializer{ p_asset_manager } {}

    virtual void serialize(const AssetMetadata& metadata, arc<IAsset>& asset) const override;
    virtual bool try_load_data(const AssetMetadata& metadata, arc<IAsset>& asset) const override;
};

// lua serializer
class lua_asset_serializer : public AssetSerializer
{
public:
    lua_asset_serializer(arc<AssetManager> p_asset_manager) : AssetSerializer{ p_asset_manager } {}

    virtual void serialize(const AssetMetadata& metadata, arc<IAsset>& asset) const override;
    virtual bool try_load_data(const AssetMetadata& metadata, arc<IAsset>& asset) const override;
};

} // end namespace kb::asset

#endif
