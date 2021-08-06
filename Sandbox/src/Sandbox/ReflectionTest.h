#pragma once

#include "Kablunk.h"

struct TestStruct
{

	uint32_t a{ 0 };
	TestStruct() = default;

	KB_REFLECT();
};
