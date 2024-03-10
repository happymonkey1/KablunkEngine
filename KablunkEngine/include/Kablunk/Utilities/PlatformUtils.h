#ifndef KABLUNK_UTILITIES_PLATFORM_UTILS_H
#define KABLUNK_UTILITIES_PLATFORM_UTILS_H

#include "Kablunk/Core/owning_buffer.h"

#include <filesystem>
#include <string>

namespace kb
{ // start namespace kb
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

// #TODO move to FileSystem file
class FileSystem
{
public:
	static bool HasEnvironmentVariable(const std::string& key);
	static std::string GetEnvironmentVar(const std::string& key);
	static bool SetEnvironmentVar(const std::string& key, const std::string& value);

	static bool file_exists(const std::filesystem::path& filepath);

    static auto read_bytes(const std::filesystem::path& p_filepath) noexcept -> owning_buffer;
};
} // end namespace kb

namespace kb::Utils
{ // start namespace kb::Utils

std::wstring StringToWideString(const std::string& str);

} // end namespace kb::Utils

#endif
