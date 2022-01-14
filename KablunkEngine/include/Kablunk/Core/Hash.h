#pragma once
#ifndef KABLUNK_CORE_HASH_H
#define KABLUNK_CORE_HASH_H

#include <stdint.h>
#include <string>

namespace Kablunk
{
	class Hash
	{
	public:
		static uint32_t GenerateFNVHash(const char* str);
		static uint32_t GenerateFNVHash(const std::string& string);
	};
}

#endif
