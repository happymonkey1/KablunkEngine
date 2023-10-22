#pragma once

#ifndef KABLUNK_ASSET_ASSET_EXTENSIONS_H
#define KABLUNK_ASSET_ASSET_EXTENSIONS_H

#include "Kablunk/Core/KablunkAPI.h"
#include "Kablunk/Asset/AssetType.h"

#include <unordered_map>

namespace kb::asset
{ // start namespace kb::asset

	inline static const kb::unordered_flat_map<std::string, AssetType> s_asset_extension_map = {
		// texture
		{ ".png",			AssetType::Texture },
		{ ".jpg",			AssetType::Texture },
		{ ".jpeg",			AssetType::Texture },
		{ ".hdr",			AssetType::Texture },

		// audio
		{ ".wav",			AssetType::Audio },
		{ ".ogg",			AssetType::Audio },
		{ ".mp3",			AssetType::Audio },
		{ ".m4a",			AssetType::Audio },

		// font
		{ ".ttf",			AssetType::Font },
		{ ".ttc",			AssetType::Font },
		{ ".otf",			AssetType::Font },

		// mesh/animation source
		{ ".fbx",			AssetType::Mesh },
		{ ".gltf",			AssetType::Mesh },
		{ ".glb",			AssetType::Mesh },
		{ ".obj",			AssetType::Mesh },
			
		// Kablunk type
		{ ".kablunkscene",	AssetType::Scene },
		{ ".kbscene",		AssetType::Scene },
		{ ".kbprefab",		AssetType::Prefab },
		{ ".kbmesh",		AssetType::Prefab },

		// Script
		{ ".h",				AssetType::NativeScript },
		{ ".hpp",			AssetType::NativeScript }
	};

} // end namespace kb::asset

#endif
