#pragma once
#ifndef KABLUNK_CORE_CORE_TYPES_H
#define KABLUNK_CORE_CORE_TYPES_H

#include <stdint.h>
#include <unordered_map>

namespace Kablunk
{
	
	using i8  = int8_t;
	using i16 = int16_t;
	using i32 = int32_t;
	using i64 = int64_t;
	
	using u8  = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;

	using f32 = float;
	using f64 = double;

	// unordered map
	template <typename K, typename V>
	using map = std::unordered_map<K, V>;
}

#endif
