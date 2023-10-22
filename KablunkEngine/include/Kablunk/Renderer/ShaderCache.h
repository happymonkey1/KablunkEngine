#pragma once
#ifndef KABLUNK_RENDERER_SHADER_CACHE_H
#define KABLUNK_RENDERER_SHADER_CACHE_H

#include <filesystem>
#include <map>

namespace kb
{
	class ShaderCache
	{
	public:
		static bool HasChanged(const std::filesystem::path& shader, const std::string& source);
	private:
		static void Serialize(const std::map<std::string, uint32_t>& shader_cache);
		static void Deserialize(std::map<std::string, uint32_t>& shader_cache);
	};
}

#endif
