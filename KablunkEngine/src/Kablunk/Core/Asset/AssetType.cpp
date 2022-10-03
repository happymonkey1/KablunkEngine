#include "kablunkpch.h"

#include "Kablunk/Asset/AssetType.h"

#include "Kablunk/Asset/AssetExtensions.h"

namespace Kablunk::asset
{

	AssetType extension_to_asset_type(const std::string& extension)
	{
		auto it = s_asset_extension_map.find(extension);
		if (it != s_asset_extension_map.end())
			return it->second;

		KB_CORE_ASSERT(false, "unknown asset extension '{}'", extension);
		return AssetType::NONE;
	}

}
