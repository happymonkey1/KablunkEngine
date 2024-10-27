#pragma once
#ifndef KABLUNK_RENDERER_SHADER_CACHE_H
#define KABLUNK_RENDERER_SHADER_CACHE_H

#include <filesystem>
#include <map>

namespace kb::render
{ // start namespace kb::render

class shader_cache
{
public:
	static bool has_changed(const std::filesystem::path& shader, const std::string& source);
private:
	static void serialize(const std::map<std::string, uint32_t>& shader_cache);
	static void deserialize(std::map<std::string, uint32_t>& shader_cache);
};

} // end namespace kb::render

#endif
