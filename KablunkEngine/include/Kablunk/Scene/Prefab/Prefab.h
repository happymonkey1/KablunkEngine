#pragma once
#ifndef KABLUNK_SCENE_PREFAB_PREFAB_H
#define KABLUNK_SCENE_PREFAB_PREFAB_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Scene/Entity.h"

namespace Kablunk
{

	class Prefab : public RefCounted
	{
	public:
		Prefab(Entity entity) noexcept;
		~Prefab();

		void create(Entity entity, bool serialize = true);
	private:
		Entity create_prefab_from_entity(Entity entity);

		void on_native_script_component_construct(entt::registry& registry, entt::entity entity);
		void on_native_script_component_destroy(entt::registry& registry, entt::entity entity);

	private:
		IntrusiveRef<Scene> m_scene_context;
		Entity m_entity;

		friend class Scene;
		friend class PrefabSerializer;
	};
}

#endif
