#include "kablunkpch.h"

#include "Kablunk/Scene/Prefab/Prefab.h"
#include "Kablunk/Scene/Components.h"
#include "Kablunk/Scene/Prefab/PrefabSerializer.h"
#include "Kablunk/Project/Project.h"

namespace Kablunk
{

	Prefab::Prefab(Entity entity) noexcept
		: m_entity{ entity }, m_scene_context{ Scene::Create() }
	{
		// register notifiers
		m_scene_context->m_registry.on_construct<NativeScriptComponent>().connect<&Prefab::on_native_script_component_construct>(this);
		m_scene_context->m_registry.on_destroy<NativeScriptComponent>().connect<&Prefab::on_native_script_component_destroy>(this);
	}

	Prefab::~Prefab()
	{
		m_scene_context->m_registry.on_construct<NativeScriptComponent>().disconnect<&Prefab::on_native_script_component_construct>(this);
		m_scene_context->m_registry.on_destroy<NativeScriptComponent>().disconnect<&Prefab::on_native_script_component_destroy>(this);
	}

	void Prefab::create(Entity entity, bool serialize /*= true*/)
	{
		m_scene_context = Scene::Create();
		m_scene_context->m_registry.on_construct<NativeScriptComponent>().connect<&Prefab::on_native_script_component_construct>(this);
		m_scene_context->m_registry.on_destroy<NativeScriptComponent>().connect<&Prefab::on_native_script_component_destroy>(this);

		m_entity = create_prefab_from_entity(entity);

		if (serialize)
		{
			PrefabSerializer serializer{ this };
			KB_CORE_ASSERT(Project::GetActive(), "No active project!");
			std::filesystem::path path = Project::GetActive()->GetAssetDirectoryPath() / "prefabs" / m_entity.GetComponent<TagComponent>().Tag;
			serializer.Serialize(path.string());
		}
	}

	Entity Prefab::create_prefab_from_entity(Entity entity)
	{
		Entity new_entity = m_scene_context->CreateEntity();

		new_entity.AddComponent<PrefabComponent>(uuid::generate(), new_entity.GetComponent<IdComponent>().Id);

		// copy components
		new_entity.m_scene->CopyComponentIfItExists<TransformComponent>(entity, new_entity, m_scene_context->m_registry);
		new_entity.m_scene->CopyComponentIfItExists<SpriteRendererComponent>(entity, new_entity, m_scene_context->m_registry);
		new_entity.m_scene->CopyComponentIfItExists<CircleRendererComponent>(entity, new_entity, m_scene_context->m_registry);
		new_entity.m_scene->CopyComponentIfItExists<CameraComponent>(entity, new_entity, m_scene_context->m_registry);
		new_entity.m_scene->CopyComponentIfItExists<NativeScriptComponent>(entity, new_entity, m_scene_context->m_registry);
		new_entity.m_scene->CopyComponentIfItExists<CSharpScriptComponent>(entity, new_entity, m_scene_context->m_registry);
		new_entity.m_scene->CopyComponentIfItExists<MeshComponent>(entity, new_entity, m_scene_context->m_registry);
		new_entity.m_scene->CopyComponentIfItExists<PointLightComponent>(entity, new_entity, m_scene_context->m_registry);
		new_entity.m_scene->CopyComponentIfItExists<RigidBody2DComponent>(entity, new_entity, m_scene_context->m_registry);
		new_entity.m_scene->CopyComponentIfItExists<BoxCollider2DComponent>(entity, new_entity, m_scene_context->m_registry);
		new_entity.m_scene->CopyComponentIfItExists<CircleCollider2DComponent>(entity, new_entity, m_scene_context->m_registry);

		// copy entity hierarchy
		auto children = entity.GetChildrenCopy();
		for (auto child_id : children)
		{
			Entity child = create_prefab_from_entity(entity.m_scene->GetEntityFromUUID(child_id));

			child.SetParentUUID(new_entity.GetUUID());
			new_entity.GetChildren().push_back(child.GetUUID());
		}

		return new_entity;
	}

	void Prefab::on_native_script_component_construct(entt::registry& registry, entt::entity entity)
	{
		registry.get<NativeScriptComponent>(entity).BindEditor();
	}

	void Prefab::on_native_script_component_destroy(entt::registry& registry, entt::entity entity)
	{

	}

}
