#pragma once
#ifndef KABLUNK_CORE_HASH_H
#define KABLUNK_CORE_HASH_H

#include "Kablunk/Core/CoreTypes.h"

#include <stdint.h>
#include <string>

namespace kb
{
	class Hash
	{
	public:
		// #TODO add constraint when KablunkEngine moves to C++20
		template <typename T = u32>
		inline static T generate_fnv_hash(const char* str)
		{
			// Based on https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
			constexpr const T FNV_PRIME = 16777619u;
			constexpr const T OFFSET_BASIS = 2166136261u;

			const size_t length = strlen(str) + 1;
			T hash = OFFSET_BASIS;
			for (size_t i = 0; i < length; ++i)
			{
				hash ^= *str++;
				hash *= FNV_PRIME;
			}
			return hash;
		}

		template <typename T = u32>
		inline static T generate_fnv_hash(const std::string& string)
		{
			return generate_fnv_hash<T>(string.c_str());
		}
	};
}

#endif
