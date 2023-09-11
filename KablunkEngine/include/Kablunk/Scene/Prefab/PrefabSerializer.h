#pragma once
#ifndef KABLUNK_SCENE_PREFAB_PREFAB_SERIALIZER_H
#define KABLUNK_SCENE_PREFAB_PREFAB_SERIALIZER_H

#include <yaml-cpp/yaml.h>

namespace Kablunk
{

	class Prefab;

	class PrefabSerializer
	{
	public:
		PrefabSerializer(const ref<Prefab>& prefab);
		~PrefabSerializer() = default;

		void Serialize(const std::string& filepath);
		void SerializeBinary(const std::string& filepath);

		bool Deserialize(const std::string& filepath);
		bool DeserializeBinary(const std::string& filepath);
	private:
		void DeserializeEntity(YAML::detail::iterator_value& entity);
	private:
		ref<Prefab> m_prefab;
	};

}

#endif
