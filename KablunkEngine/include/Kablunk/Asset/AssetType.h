#pragma once

#ifndef KABLUNK_ASSET_ASSET_TYPE_H
#define KABLUNK_ASSET_ASSET_TYPE_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/CoreTypes.h"

#include "Kablunk/Asset/AssetTypeDefs.h"

#include <string>

namespace Kablunk::asset
{

	// #TODO: only 255 different asset types supports
	enum class AssetType : u8
	{
		Texture = 0,
		Mesh,
		MeshSource,
		Audio,
		Prefab,
		Material,
		NativeScript,
		Font,
		Scene,
		NONE
	};

	struct AssetTypeStrings
	{
		DEFINE_ENUM_TYPE_STRING(Texture,		"Texture");
		DEFINE_ENUM_TYPE_STRING(Mesh,			"Mesh");
		DEFINE_ENUM_TYPE_STRING(MeshSource,		"MeshSource");
		DEFINE_ENUM_TYPE_STRING(Audio,			"Audio");
		DEFINE_ENUM_TYPE_STRING(Prefab,			"Prefab");
		DEFINE_ENUM_TYPE_STRING(Material,		"Material");
		DEFINE_ENUM_TYPE_STRING(NativeScript,	"NativeScript");
		DEFINE_ENUM_TYPE_STRING(Font,			"Font");
		DEFINE_ENUM_TYPE_STRING(Scene,			"Scene");
		DEFINE_ENUM_TYPE_STRING(NONE,			"INV_ASSET_TYPE");
	};

	// Get an AssetType from a string
	inline AssetType string_to_asset_type(const std::string& asset_type_str)
	{
		if (asset_type_str == AssetTypeStrings::Texture)			return AssetType::Texture;
		else if (asset_type_str == AssetTypeStrings::Mesh)			return AssetType::Mesh;
		else if (asset_type_str == AssetTypeStrings::MeshSource)	return AssetType::MeshSource;
		else if (asset_type_str == AssetTypeStrings::Audio)			return AssetType::Audio;
		else if (asset_type_str == AssetTypeStrings::Prefab)		return AssetType::Prefab;
		else if (asset_type_str == AssetTypeStrings::Material)		return AssetType::Material;
		else if (asset_type_str == AssetTypeStrings::NativeScript)	return AssetType::NativeScript;
		else if (asset_type_str == AssetTypeStrings::Font)			return AssetType::Font;
		else if (asset_type_str == AssetTypeStrings::Scene)			return AssetType::Scene;
		else if (asset_type_str == AssetTypeStrings::NONE)			return AssetType::NONE;
		else
		{
			KB_CORE_ASSERT(false, "AssetType not implemented!");
			return AssetType::NONE;
		}
	}

	inline const char* asset_type_to_string(AssetType asset_type)
	{
		switch (asset_type)
		{
			case AssetType::Texture:		return AssetTypeStrings::Texture;
			case AssetType::Mesh:			return AssetTypeStrings::Mesh;
			case AssetType::MeshSource:		return AssetTypeStrings::MeshSource;
			case AssetType::Audio:			return AssetTypeStrings::Audio;
			case AssetType::Prefab:			return AssetTypeStrings::Prefab;
			case AssetType::Material:		return AssetTypeStrings::Material;
			case AssetType::NativeScript:	return AssetTypeStrings::NativeScript;
			case AssetType::Font:			return AssetTypeStrings::Font;
			case AssetType::Scene:			return AssetTypeStrings::Scene;
			case AssetType::NONE:			KB_CORE_ASSERT(false, "Invalid AssetType!"); return AssetTypeStrings::NONE;
			default:						KB_CORE_ASSERT(false, "Unhandled AssetType"); return AssetTypeStrings::NONE;
		}
	}
}

#endif
