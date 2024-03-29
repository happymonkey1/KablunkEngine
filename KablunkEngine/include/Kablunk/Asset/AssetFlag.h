#pragma once

#ifndef KABLUNK_ASSET_ASSET_FLAG_H
#define KABLUNK_ASSET_ASSET_FLAG_H

#include "Kablunk/Core/KablunkAPI.h"
#include "Kablunk/Core/CoreTypes.h"

namespace kb::asset
{

	enum class AssetFlag : u8
	{
		NONE = 0,
		Invalid = 0b01,
		Missing = 0b10,
	};

}

#endif
