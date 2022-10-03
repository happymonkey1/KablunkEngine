#pragma once

#ifndef KABLUNK_ASSET_ASSET_TYPE_DEFS_H
#define KABLUNK_ASSET_ASSET_TYPE_DEFS_H

#include <Kablunk/Core/Uuid64.h>

#include "Kablunk/Asset/AssetFlag.h"

namespace Kablunk::asset
{
	using asset_id_t = uuid::uuid64;
	static constexpr const asset_id_t null_asset_id = uuid::nil_uuid;
	
	using asset_type_t = AssetType;
	using asset_flag_t = AssetFlag;
}

#endif
