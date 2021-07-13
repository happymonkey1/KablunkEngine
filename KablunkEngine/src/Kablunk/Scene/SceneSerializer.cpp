#include "kablunkpch.h"

#include "Kablunk/Scene/SceneSerializer.h"

#include "Kablunk/Scene/YamlSpecializedSerialization.h"
#include "Kablunk/Scene/Entity.h"
#include "Kablunk/Scene/Components.h"

#include <fstream>

namespace Kablunk
{

	// #TODO almost all of the serialization code is garbage, just there for an MVP
	// please refactor to use reflection.

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_scene{ scene }
	{
		
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& member)
	{
		out << YAML::Flow << YAML::BeginSeq;
		out << member.x << member.y << member.z;
		out << YAML::EndSeq;

		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& member)
	{
		out << YAML::Flow << YAML::BeginSeq;
		out << member.x << member.y << member.z << member.w;
		out << YAML::EndSeq;

		return out;
	}

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
			out << YAML::Key << "Translation"	<< YAML::Value << transform.Translation;
			out << YAML::Key << "Rotation"		<< YAML::Value << transform.Rotation;
			out << YAML::Key << "Scale"			<< YAML::Value << transform.Scale;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;

			auto& comp = entity.GetComponent<CameraComponent>();

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Kablunk::Camera
			
			const auto& camera = comp.Camera;
			out << YAML::Key << "m_projection_type"		<< YAML::Value << static_cast<uint32_t>(camera.GetProjectionType());

			out << YAML::Key << "m_perspective_fov"		<< YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "m_perspective_near"	<< YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "m_perspective_far"		<< YAML::Value << camera.GetPerspectiveFarClip();
			
			out << YAML::Key << "m_orthographic_size"	<< YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "m_orthographic_near"	<< YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "m_orthographic_far"	<< YAML::Value << camera.GetOrthographicFarClip();

			out << YAML::Key << "m_aspect_ratio"		<< YAML::Value << camera.GetAspectRatio();

			out << YAML::EndMap;

			out << YAML::Key << "Primary"				<< YAML::Value << comp.Primary;
			out << YAML::Key << "Fixed_aspect_ratio"	<< YAML::Value << comp.Fixed_aspect_ratio;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			auto& comp = entity.GetComponent<SpriteRendererComponent>();
			
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap;

			out << YAML::Key << "Texture"		<< YAML::Value << "AssetID HERE";
			out << YAML::Key << "Color"			<< YAML::Value << comp.Color;
			out << YAML::Key << "Tiling_factor" << YAML::Value << comp.Tiling_factor;

			out << YAML::EndMap;
		}
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap;
		// #TODO add entity id instead of random number
		out << YAML::Key << "Entity" << YAML::Value << "654621321";

		SerializeComponents(out, entity);

		out << YAML::EndMap; 
	}

	void SceneSerializer::DeserializeEntity(YAML::detail::iterator_value& entity)
	{
		uint64_t uuid = entity["Entity"].as<uint64_t>();

		std::string name;
		auto tag_comp = entity["TagComponent"];
		if (tag_comp)
			name = tag_comp["Tag"].as<std::string>();

		KB_CORE_TRACE("Deserializing entity with ID: {0}, name: {1}", uuid, name);
		KB_CORE_WARN("uuid not passed in when deserializing entity!");
		Entity deserialized_entity = m_scene->CreateEntity(name);

		// Beyond this is pretty much garbage just to have a 'usable' deserialization feature

		auto transform_data = entity["TransformComponent"];
		if (transform_data)
		{
			auto& transform = deserialized_entity.GetOrAddComponent<TransformComponent>();
			transform.Translation	= transform_data["Translation"].as<glm::vec3>();
			transform.Rotation		= transform_data["Rotation"].as<glm::vec3>();
			transform.Scale			= transform_data["Scale"].as<glm::vec3>();
		}

		auto camera_data = entity["CameraComponent"];
		if (camera_data)
		{
			auto& camera_comp = deserialized_entity.GetOrAddComponent<CameraComponent>();
			
			auto& scene_camera_data = camera_data["Camera"];
			if (scene_camera_data)
			{
				auto scene_camera = SceneCamera{};
				
				auto perspective_fov	= scene_camera_data["m_perspective_fov"].as<float>();
				auto perspective_near	= scene_camera_data["m_perspective_near"].as<float>();
				auto perspective_far	= scene_camera_data["m_perspective_far"].as<float>();

				auto orthographic_size	= scene_camera_data["m_orthographic_size"].as<float>();
				auto orthographic_near	= scene_camera_data["m_orthographic_near"].as<float>();
				auto orthographic_far	= scene_camera_data["m_orthographic_far"].as<float>();

				auto aspect_ratio		= scene_camera_data["m_aspect_ratio"].as<float>();
				auto projection_type	= scene_camera_data["m_projection_type"].as<int>();

				scene_camera.SetOrthographic(orthographic_size, orthographic_near, orthographic_far);
				scene_camera.SetPerspective(perspective_far, perspective_near, perspective_far);
				scene_camera.SetProjectionType(static_cast<SceneCamera::ProjectionType>(projection_type));
				scene_camera.SetAspectRatio(aspect_ratio);

				camera_comp.Camera = scene_camera;
			}

			camera_comp.Primary				= camera_data["Primary"].as<bool>();
			camera_comp.Fixed_aspect_ratio	= camera_data["Fixed_aspect_ratio"].as<bool>();
		}

		auto sprite_renderer_data = entity["SpriteRendererComponent"];
		if (sprite_renderer_data)
		{
			auto& sprite_renderer_comp = deserialized_entity.GetOrAddComponent<SpriteRendererComponent>();
			
			KB_CORE_ERROR("Entity with ID: {0}, SpriteRendererComponent Texture not set during deserialization!", uuid);
			//sprite_renderer_comp.Texture = transform_data["Translation"].as<glm::vec3>();
			sprite_renderer_comp.Color			= sprite_renderer_data["Color"].as<glm::vec4>();
			sprite_renderer_comp.Tiling_factor = sprite_renderer_data["Tiling_factor"].as<float>();
		}
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
		std::ifstream stream{ filepath };
		std::stringstream sstream;
		sstream << stream.rdbuf();

		YAML::Node root = YAML::Load(sstream.str());
		if (!root["Scene"])
			return false;

		std::string scene_name = root["Scene"].as<std::string>();
		KB_CORE_TRACE("Deserializing scene '{0}'", scene_name);

		auto entities = root["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				DeserializeEntity(entity);
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeBinary(const std::string& filepath)
	{
		KB_CORE_ASSERT(false, "Deserializing binary not implemented!");
		return false;
	}

}
