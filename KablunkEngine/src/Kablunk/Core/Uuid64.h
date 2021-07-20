#ifndef KABLUNK_CORE_UUID64_H
#define KABLUNK_CORE_UUID64_H

#include "Kablunk/Core/Log.h"

#include <random>
#include <limits>
#include <string>
#include <sstream>

#include "Kablunk/Utilities/PlatformUtils.h"

namespace Kablunk
{
	namespace uuid
	{
		using uuid64 = uint64_t;

		static std::random_device rd;
		constexpr uint64_t uint64_t_max = std::numeric_limits<uint64_t>::max();

		static std::string to_string(uuid64 id)
		{
			auto ss = std::stringstream{};
			ss << std::hex << id;
			return "0x" + ss.str();
		}

		static uuid64 generate()
		{
			auto twister = std::mt19937_64{ rd() };
			auto dist = std::uniform_int_distribution<uint64_t>{ 1, uint64_t_max };

			uuid64 id = dist(twister);

			uint64_t random_mask = 0xFFFF;
			uint64_t random_bits = id & random_mask;
			auto mac_address = MacAddress::Get();

			return (mac_address << 16) | random_bits;
		}
	}
}

#endif
