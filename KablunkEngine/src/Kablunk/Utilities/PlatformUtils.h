#ifndef KABLUNK_UTILITIES_PLATFORM_UTILS_H
#define KABLUNK_UTILITIES_PLATFORM_UTILS_H

#include <string>

namespace Kablunk
{
	class FileDialog
	{
	public:
		static std::string OpenFile(const char* filter);
		static std::string OpenFolder(const char* starting_folder = "");
		static std::string SaveFile(const char* filter);
	};

	class MacAddress
	{
	public:
		static uint64_t Get();
	};
}

#endif
