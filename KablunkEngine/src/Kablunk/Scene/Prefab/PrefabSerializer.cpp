#include "kablunkpch.h"

#include "Kablunk/Scene/Prefab/PrefabSerializer.h"
#include "Kablunk/Scene/Prefab/Prefab.h"

namespace Kablunk
{

	PrefabSerializer::PrefabSerializer(const IntrusiveRef<Prefab>& prefab)
		: m_prefab{ prefab }
	{

	}

	void PrefabSerializer::Serialize(const std::string& filepath)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void PrefabSerializer::SerializeBinary(const std::string& filepath)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	bool PrefabSerializer::Deserialize(const std::string& filepath)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return false;
	}

	bool PrefabSerializer::DeserializeBinary(const std::string& filepath)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return false;
	}

	void PrefabSerializer::DeserializeEntity(YAML::detail::iterator_value& entity)
	{

	}

}
