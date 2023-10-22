#pragma once
#ifndef KABLUNK_CORE_FILE_FILE_STATUS_H
#define KABLUNK_CORE_FILE_FILE_STATUS_H

#include <stdint.h>

namespace kb
{
	enum class FileStatus : uint8_t
	{
		created,
		modified,
		erased
	};
}

#endif
