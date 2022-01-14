#include "kablunkpch.h"

#include "Kablunk/Renderer/ShaderCache.h"
#include "Kablunk/Core/Hash.h"

#include <yaml-cpp/yaml.h>

namespace Kablunk
{
	static const char* s_shader_registry_path = "resources/cache/shader/shader_registry.cache";

	bool ShaderCache::HasChanged(const std::filesystem::path& shader, const std::string& source)
	{
		std::map<std::string, uint32_t> shader_cache;
		Deserialize(shader_cache);

		uint32_t hash = Hash::GenerateFNVHash(source.c_str());
		if (shader_cache.find(shader.string()) == shader_cache.end() || shader_cache.at(shader.string()) != hash)
		{
			shader_cache[shader.string()] = hash;
			Serialize(shader_cache);
			return true;
		}

		return false;
	}

	void ShaderCache::Serialize(const std::map<std::string, uint32_t>& shader_cache)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "shader_registry" << YAML::BeginMap;

		for (auto& [filepath, hash] : shader_cache)
			out << YAML::Key << filepath << YAML::Value << hash;

		out << YAML::EndMap;
		out << YAML::EndMap;

		std::ofstream fout(s_shader_registry_path);
		fout << out.c_str();

	}

	void ShaderCache::Deserialize(std::map<std::string, uint32_t>& shader_cache)
	{
		// make sure file exists
		std::ifstream stream(s_shader_registry_path);
		if (!stream.good())
			return;

		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		auto handles = data["shader_registry"];
		if (!handles)
		{
			KB_CORE_ERROR("[ShaderCache] Shader Registry is invalid.");
			return;
		}

		for (auto entry : handles)
		{
			std::string path = entry.first.as<std::string>();
			uint32_t hash = entry.second.as<uint32_t>();
			shader_cache[path] = hash;
		}

	}

}
