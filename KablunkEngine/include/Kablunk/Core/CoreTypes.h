#pragma once
#ifndef KABLUNK_CORE_CORE_TYPES_H
#define KABLUNK_CORE_CORE_TYPES_H

#include <stdint.h>
#include <unordered_map>
#pragma warning(push, 0)
#include <robin-hood/robin_hood.h>
#pragma warning(pop)

namespace kb
{ // start namespace kb
	
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

    // unordered flat map
    template <typename K, typename V>
    using unordered_flat_map = robin_hood::unordered_flat_map<K, V>;
} // end namespace kb

#endif
