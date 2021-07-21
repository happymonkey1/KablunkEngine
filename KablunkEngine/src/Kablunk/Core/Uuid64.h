#ifndef KABLUNK_CORE_UUID64_H
#define KABLUNK_CORE_UUID64_H

#include "Kablunk/Core/Log.h"

#include <random>
#include <limits>
#include <string>
#include <sstream>

#include "Kablunk/Utilities/PlatformUtils.h"

// Define KB_TRUE_UUID to use actual mac in uuid value instead of as a modifier.
// Doing this will mean that the uuids will currently only have 16 bits of entropy
#if 0
#	define KB_TRUE_UUID
#endif

namespace Kablunk
{
	namespace uuid
	{
		using uuid64 = uint64_t;
		constexpr uuid64 nil_uuid = 0;

		static std::random_device rd;
		constexpr uint64_t uint64_t_max = std::numeric_limits<uint64_t>::max();

		static std::string to_string(uuid64 id)
		{
			auto ss = std::stringstream{};
			ss << std::hex << id;
			return "0x" + ss.str();
		}

		// #TODO add more bits of entropy, currently only 16!
		static uuid64 generate()
		{
			// Should probably make these static or find another way of doing this
			// Creating a generator and distribution every time is probably not the most performant
			auto twister = std::mt19937_64{ rd() };
			auto dist = std::uniform_int_distribution<uint64_t>{ 1, uint64_t_max };

			uuid64 id = dist(twister);
			uint64_t mac_address = MacAddress::Get();
#ifdef KB_TRUE_UUID
			KB_CORE_WARN("uuid generator currently has 16 bits of entropy!");
			uint64_t random_mask = 0xFFFF;
			uint64_t random_bits = id & random_mask;
#endif
			return (mac_address << 16) ^ id;
		}

		static bool is_nil(uuid64 id) { return id == nil_uuid; }
	}
}

#endif
