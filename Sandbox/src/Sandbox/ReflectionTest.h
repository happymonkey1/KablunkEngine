#pragma once

#include <Kablunk.h>

struct TestStruct
{

	uint32_t a{ 0 };
	uint32_t b{ 1 };
	uint32_t c{ 2 };
	bool     d{ false };
	TestStruct() = default;

	KB_REFLECT();
};
