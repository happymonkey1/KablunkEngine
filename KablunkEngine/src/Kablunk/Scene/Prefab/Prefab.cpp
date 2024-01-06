#include "kablunkpch.h"

#include "Kablunk/Scene/Prefab/Prefab.h"
#include "Kablunk/Scene/Components.h"
#include "Kablunk/Scene/Prefab/PrefabSerializer.h"
#include "Kablunk/Project/ProjectManager.h"

namespace kb
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
        PrefabSerializer serializer{ ref{ this } };
		KB_CORE_ASSERT(ProjectManager::get().get_active(), "No active project!");
		const std::filesystem::path path = ProjectManager::get().get_active()->get_asset_directory_path() / "prefabs" / m_entity.GetComponent<TagComponent>().Tag;
		serializer.Serialize(path.string());
	}
}

Entity Prefab::create_prefab_from_entity(Entity entity)
{
	Entity new_entity = m_scene_context->CreateEntity();

	auto& id = new_entity.GetComponent<IdComponent>().Id;
	new_entity.AddComponent<PrefabComponent>(uuid::generate(), id);

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
	const auto children = entity.GetChildrenCopy();
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
