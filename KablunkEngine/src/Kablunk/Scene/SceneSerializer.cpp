#include "kablunkpch.h"

#include "Kablunk/Scene/SceneSerializer.h"

#include "Kablunk/Scene/YamlSpecializedSerialization.h"
#include "Kablunk/Scene/Entity.h"
#include "Kablunk/Scene/Components.h"

#include <fstream>
#include <typeinfo>

namespace Kablunk
{

	// #FIXME almost all of the serialization code is garbage, just there for an MVP
	// #TODO refactor to use reflection.

	SceneSerializer::SceneSerializer(const IntrusiveRef<Scene>& scene)
		: m_scene{ scene }
	{
		
	}

	template <typename ComponentT, typename WriteDataFunc>
	static void WriteComponentData(YAML::Emitter& out, Entity entity, WriteDataFunc WriteMemberData)
	{
		if (entity.HasComponent<ComponentT>())
		{
			out << YAML::Key << Parser::String::DemangleTypeIDName<ComponentT>();
			out << YAML::BeginMap;

			auto& component = entity.GetComponent<ComponentT>();
			WriteMemberData(out, component);

			out << YAML::EndMap;
		}
	}

	static void SerializeComponents(YAML::Emitter& out, Entity entity)
	{

		WriteComponentData<TagComponent>(out, entity, [](auto& out, auto& component)
			{
				auto& tag = component.Tag;
				out << YAML::Key << "Tag" << YAML::Value << tag;
			});

		WriteComponentData<TransformComponent>(out, entity, [](auto& out, auto& component)
			{
				out << YAML::Key << "Translation"	<< YAML::Value << component.Translation;
				out << YAML::Key << "Rotation"		<< YAML::Value << component.Rotation;
				out << YAML::Key << "Scale"			<< YAML::Value << component.Scale;
			});

		WriteComponentData<CameraComponent>(out, entity, [](auto& out, auto& component)
			{
				const auto& camera = component.Camera;

				out << YAML::Key << "Camera" << YAML::Value;
				out << YAML::BeginMap; // Camera

				out << YAML::Key << "m_projection_type" << YAML::Value << static_cast<uint32_t>(camera.GetProjectionType());

				out << YAML::Key << "m_perspective_fov" << YAML::Value << camera.GetPerspectiveVerticalFOV();
				out << YAML::Key << "m_perspective_near" << YAML::Value << camera.GetPerspectiveNearClip();
				out << YAML::Key << "m_perspective_far" << YAML::Value << camera.GetPerspectiveFarClip();

				out << YAML::Key << "m_orthographic_size" << YAML::Value << camera.GetOrthographicSize();
				out << YAML::Key << "m_orthographic_near" << YAML::Value << camera.GetOrthographicNearClip();
				out << YAML::Key << "m_orthographic_far" << YAML::Value << camera.GetOrthographicFarClip();

				out << YAML::Key << "m_aspect_ratio" << YAML::Value << camera.GetAspectRatio();

				out << YAML::EndMap;

				out << YAML::Key << "Primary" << YAML::Value << component.Primary;
				out << YAML::Key << "Fixed_aspect_ratio" << YAML::Value << component.Fixed_aspect_ratio;
			});

		WriteComponentData<SpriteRendererComponent>(out, entity, [](auto& out, auto& component)
			{
				out << YAML::Key << "Texture" << YAML::Value;

				out << YAML::BeginMap; // Texture Asset

				out << YAML::Key << "m_uuid"		<< YAML::Value << component.Texture.GetUUID();
				out << YAML::Key << "m_filepath"	<< YAML::Value << component.Texture.GetFilepath();

				out << YAML::EndMap;

				out << YAML::Key << "Color"			<< YAML::Value << component.Color;
				out << YAML::Key << "Tiling_factor" << YAML::Value << component.Tiling_factor;
			});

		WriteComponentData<CircleRendererComponent>(out, entity, [](auto& out, CircleRendererComponent& component)
			{
				out << YAML::Key << "Color" << YAML::Value << component.Color;
				out << YAML::Key << "Radius" << YAML::Value << component.Radius;
				out << YAML::Key << "Thickness" << YAML::Value << component.Thickness;
				out << YAML::Key << "Fade" << YAML::Value << component.Fade;
			});
#if KB_NATIVE_SCRIPTING
		WriteComponentData<NativeScriptComponent>(out, entity, [](auto& out, auto& component)
			{
				out << YAML::Key << "Filepath" << YAML::Value << component.Filepath.string();
			});
#endif

		WriteComponentData<CSharpScriptComponent>(out, entity, [](auto& out, CSharpScriptComponent& component)
			{
				out << YAML::Key << "Module_name" << YAML::Value << component.Module_name;
			});

		WriteComponentData<MeshComponent>(out, entity, [](auto& out, auto& component)
			{
				out << YAML::Key << "Filepath" << YAML::Value << component.Filepath;
			});

		WriteComponentData<PointLightComponent>(out, entity, [](auto& out, auto& component)
			{
				out << YAML::Key << "Multiplier" << YAML::Value << component.Multiplier;
				out << YAML::Key << "Radiance" << YAML::Value << component.Radiance;
				out << YAML::Key << "Radius" << YAML::Value << component.Radius;
				out << YAML::Key << "Min_radius" << YAML::Value << component.Min_radius;
				out << YAML::Key << "Falloff" << YAML::Value << component.Falloff;
			});

		WriteComponentData<RigidBody2DComponent>(out, entity, [](auto& out, RigidBody2DComponent& component)
			{
				out << YAML::Key << "Type" << YAML::Value << static_cast<int32_t>(component.Type);
				out << YAML::Key << "Fixed_rotation" << YAML::Value << component.Fixed_rotation;
				out << YAML::Key << "Does_gravity_affect" << YAML::Value << component.Does_gravity_affect;
			});

		WriteComponentData<BoxCollider2DComponent>(out, entity, [](auto& out, BoxCollider2DComponent& component)
			{
				out << YAML::Key << "Offset" << YAML::Value << component.Offset;
				out << YAML::Key << "Size" << YAML::Value << component.Size;
				out << YAML::Key << "Density" << YAML::Value << component.Density;
				out << YAML::Key << "Friction" << YAML::Value << component.Friction;
				out << YAML::Key << "Restitution" << YAML::Value << component.Restitution;
				out << YAML::Key << "Restitution_threshold" << YAML::Value << component.Restitution_threshold;
			});

		WriteComponentData<CircleCollider2DComponent>(out, entity, [](auto& out, CircleCollider2DComponent& component)
			{
				out << YAML::Key << "Offset" << YAML::Value << component.Offset;
				out << YAML::Key << "Radius" << YAML::Value << component.Radius;
				out << YAML::Key << "Density" << YAML::Value << component.Density;
				out << YAML::Key << "Friction" << YAML::Value << component.Friction;
				out << YAML::Key << "Restitution" << YAML::Value << component.Restitution;
				out << YAML::Key << "Restitution_threshold" << YAML::Value << component.Restitution_threshold;
			});
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap;
		
		if (entity.HasComponent<IdComponent>())
		{
			auto& id_comp = entity.GetComponent<IdComponent>();
			if (id_comp)
			{
				auto& uuid = id_comp.Id;
				out << YAML::Key << "Entity" << YAML::Value << static_cast<uint64_t>(uuid);

				SerializeComponents(out, entity);
			}
		}

		out << YAML::EndMap; 
	}

	template <typename ComponentT, typename ReadDataFunc>
	static void ReadComponentData(const YAML::detail::iterator_value& data, Entity entity, ReadDataFunc job_func)
	{
		auto comp_data = data[Parser::String::DemangleTypeIDName<ComponentT>()];
		if (comp_data)
		{
			auto& component = entity.GetOrAddComponent<ComponentT>();
			job_func(component, comp_data);
		}
	}

	void SceneSerializer::DeserializeEntity(YAML::detail::iterator_value& entity_data)
	{
		auto uuid = entity_data["Entity"].as<uuid::uuid64>();



		std::string name;
		auto tag_comp = entity_data["TagComponent"];
		if (tag_comp)
			name = tag_comp["Tag"].as<std::string>();

#ifdef KB_DEBUG
		KB_CORE_TRACE("Deserializing entity with ID: {0}, name: {1}", uuid, name);
#endif

		Entity entity = m_scene->CreateEntity(name, uuid);

		// #FIXME Beyond this is pretty much boilerplate garbage code just to have a 'usable' deserialization feature

		ReadComponentData<TransformComponent>(entity_data, entity, [this](auto& component, auto& data)
			{
				component.Translation	= data["Translation"].as<glm::vec3>();
				component.Rotation		= data["Rotation"].as<glm::vec3>();
				component.Scale			= data["Scale"].as<glm::vec3>();
			});

		ReadComponentData<CameraComponent>(entity_data, entity, [this](auto& component, auto& data)
			{
				auto& scene_camera_data = data["Camera"];
				if (scene_camera_data)
				{
					auto scene_camera = SceneCamera{};

					auto perspective_fov = scene_camera_data["m_perspective_fov"].as<float>();
					auto perspective_near = scene_camera_data["m_perspective_near"].as<float>();
					auto perspective_far = scene_camera_data["m_perspective_far"].as<float>();

					auto orthographic_size = scene_camera_data["m_orthographic_size"].as<float>();
					auto orthographic_near = scene_camera_data["m_orthographic_near"].as<float>();
					auto orthographic_far = scene_camera_data["m_orthographic_far"].as<float>();

					auto aspect_ratio = scene_camera_data["m_aspect_ratio"].as<float>();
					auto projection_type = scene_camera_data["m_projection_type"].as<int>();

					scene_camera.SetPerspectiveVerticalFOV(perspective_fov);
					scene_camera.SetOrthographic(orthographic_size, orthographic_near, orthographic_far);
					scene_camera.SetPerspective(perspective_far, perspective_near, perspective_far);
					scene_camera.SetProjectionType(static_cast<SceneCamera::ProjectionType>(projection_type));
					scene_camera.SetAspectRatio(aspect_ratio);


					component.Camera = scene_camera;
				}

				component.Primary = data["Primary"].as<bool>();
				component.Fixed_aspect_ratio = data["Fixed_aspect_ratio"].as<bool>();
			});

		ReadComponentData<SpriteRendererComponent>(entity_data, entity, [this](auto& component, auto& data) {
			
			auto texture_data = data["Texture"];
			if (texture_data)
			{
				auto uuid = texture_data["m_uuid"].as<uint64_t>();
				auto filepath = texture_data["m_filepath"].as<std::string>();

				auto texture_asset = Asset<Texture2D>(filepath, uuid);
				component.Texture = texture_asset;
			}

			component.Color = data["Color"].as<glm::vec4>();
			component.Tiling_factor = data["Tiling_factor"].as<float>();
			
			});

		ReadComponentData<CircleRendererComponent>(entity_data, entity, [this](CircleRendererComponent& component, auto& data) 
			{
				component.Color		= data["Color"].as<glm::vec4>();
				component.Radius	= data["Radius"].as<float>();
				component.Thickness = data["Thickness"].as<float>();
				component.Fade		= data["Fade"].as<float>();
			});
#if KB_NATIVE_SCRIPTING
		ReadComponentData<NativeScriptComponent>(entity_data, entity, [&](auto& component, auto& data)
			{
				auto filepath = data["Filepath"].as<std::string>();
				component.BindEditor(filepath, entity);

				KB_CORE_ASSERT(!component.Filepath.empty(), "Deserialized Entity '{0}' loaded script component with empty filepath!", uuid);
			});
#endif

		/*ReadComponentData<CSharpScriptComponent>(entity_data, entity, [&](CSharpScriptComponent& component, auto& data)
			{
				component.Module_name = data["Module_name"].as<std::string>();
			});*/
		auto script_comp = entity_data["CSharpScriptComponent"];
		if (script_comp)
		{
			auto module_name = script_comp["Module_name"].as<std::string>();
			CSharpScriptComponent& csharp_script_comp = entity.AddComponent<CSharpScriptComponent>(module_name);
		}

		ReadComponentData<MeshComponent>(entity_data, entity, [&](auto& component, auto& data)
			{
				auto filepath = data["Filepath"].as<std::string>();
				component.LoadMeshFromFileEditor(filepath, entity);

				KB_CORE_ASSERT(!component.Filepath.empty(), "Deserialized Entity '{0}' loaded mesh component with empty filepath!", uuid);
			});

		ReadComponentData<PointLightComponent>(entity_data, entity, [&](auto& component, auto& data)
			{
				component.Multiplier	= data["Multiplier"].as<float>();
				component.Radiance		= data["Radiance"].as<glm::vec3>();
				component.Radius		= data["Radius"].as<float>();
				component.Min_radius	= data["Min_radius"].as<float>();
				component.Falloff		= data["Falloff"].as<float>();
			});

		ReadComponentData<RigidBody2DComponent>(entity_data, entity, [&](RigidBody2DComponent& component, auto& data)
			{
				component.Type = static_cast<RigidBody2DComponent::RigidBodyType>(data["Type"].as<int32_t>());
				component.Fixed_rotation = data["Fixed_rotation"].as<bool>();
				component.Does_gravity_affect = data["Does_gravity_affect"].as<bool>();
			});

		ReadComponentData<BoxCollider2DComponent>(entity_data, entity, [&](BoxCollider2DComponent& component, auto& data)
			{
				component.Offset = data["Offset"].as<glm::vec2>();
				component.Size = data["Size"].as<glm::vec2>();
				component.Density = data["Density"].as<float>();
				component.Friction = data["Friction"].as<float>();
				component.Restitution = data["Restitution"].as<float>();
				component.Restitution_threshold = data["Restitution_threshold"].as<float>();
			});

		ReadComponentData<CircleCollider2DComponent>(entity_data, entity, [&](CircleCollider2DComponent& component, auto& data)
			{
				component.Offset = data["Offset"].as<glm::vec2>();
				component.Radius = data["Radius"].as<float>();
				component.Density = data["Density"].as<float>();
				component.Friction = data["Friction"].as<float>();
				component.Restitution = data["Restitution"].as<float>();
				component.Restitution_threshold = data["Restitution_threshold"].as<float>();
			});
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		
		std::string scene_name;
		if (m_scene->m_name != DEFAULT_SCENE_NAME)
			scene_name = m_scene->m_name;
		else
		{
			std::filesystem::path filepath_as_path{ filepath };
			scene_name = filepath_as_path.stem().string();
			m_scene->m_name = scene_name;
		}
		
		out << YAML::Key << "Scene"		<< YAML::Value << scene_name;
		out << YAML::Key << "Entities"	<< YAML::Value << YAML::BeginSeq;

		/*m_scene->m_registry.each([&](auto entity_handle)
			{
				auto entity = Entity{ entity_handle, m_scene.get() };
				if (!entity)
					return;

				SerializeEntity(out, entity);
			});*/
		auto view = m_scene->m_registry.view<IdComponent>();
		for (auto e : view)
			SerializeEntity(out, { e, m_scene.get() });

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

		YAML::Node root;
		try
		{
			root = YAML::Load(sstream.str());
		}
		catch (YAML::ParserException& e)
		{
			KB_CORE_ERROR("Failed to deserialize scene file '{0}'", filepath);
			KB_CORE_ERROR("    {0}", e.msg);
			return false;
		}

		if (!root["Scene"])
			return false;

		std::string scene_name = root["Scene"].as<std::string>();
		KB_CORE_TRACE("Deserializing scene '{0}'", scene_name);
		m_scene->m_name = scene_name;


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
