#include "kablunkpch.h"
#include "Kablunk/Scene/SceneSerializer.h"

#include "Kablunk/Scene/Entity.h"
#include "Kablunk/Scene/Components.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Kablunk
{

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_scene{ scene }
	{

	}

	// #TODO refactor to use reflection instead of hard coding you lazy fuck
	template <typename T>
	static void SerializeType(YAML::Emitter& out, const std::string& label, const T& member, char separator = ',')
	{
		KB_CORE_ASSERT(false, "default serialize type not implemented");
	}

	// #TODO refactor to use reflection instead of hard coding you lazy fuck
	template <>
	static void SerializeType<glm::vec3>(YAML::Emitter& out, const std::string& label, const glm::vec3& member, char separator)
	{
		std::string vector_as_str = std::to_string(member.x) + separator + std::to_string(member.y) + separator + std::to_string(member.z);
		out << YAML::Key << label << YAML::Value << vector_as_str;
	}

	// #TODO refactor to use reflection instead of hard coding you lazy fuck
	static void SerializeComponents(YAML::Emitter& out, Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;

			auto& transform = entity.GetComponent<TransformComponent>();
			SerializeType(out, "Translation", transform.Translation);
			SerializeType(out, "Rotation", transform.Rotation);
			SerializeType(out, "Scale", transform.Scale);

			out << YAML::EndMap;
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;

			auto& transform = entity.GetComponent<TransformComponent>();
			SerializeType(out, "Translation", transform.Translation);
			SerializeType(out, "Rotation", transform.Rotation);
			SerializeType(out, "Scale", transform.Scale);

			out << YAML::EndMap;
		}
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap;
		// #TODO add entity id instead of random number
		out << YAML::Key << "Entity" << YAML::Value << "EntityID HERE";

		SerializeComponents(out, entity);

		out << YAML::EndMap; 
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene"		<< YAML::Value << "Untitled";
		out << YAML::Key << "Entities"	<< YAML::Value << YAML::BeginSeq;

		m_scene->m_registry.each([&](auto entity_handle) 
			{
				auto entity = Entity{ entity_handle, m_scene.get() };
				if (!entity)
					return;

				SerializeEntity(out, entity);
			});

		out << YAML::EndSeq;
		out << YAML::EndMap;

		// Write to file
		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeBinary(const std::string& filepath)
	{
		KB_CORE_ASSERT(false, "Serializing binary not implemented!");
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		return false;
	}

	bool SceneSerializer::DeserializeBinary(const std::string& filepath)
	{
		KB_CORE_ASSERT(false, "Deserializing binary not implemented!");
		return false;
	}

}
