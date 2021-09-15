#ifndef KABLUNK_SCENE_SCENE_H
#define KABLUNK_SCENE_SCENE_H

#include "entt.hpp"


#include "Kablunk/Core/Timestep.h"
#include "Kablunk/Core/Uuid64.h"
#include "Kablunk/Renderer/EditorCamera.h"

namespace Kablunk
{
	class Entity;

	
	using EntityMap = std::unordered_map<uuid::uuid64, Entity>;

	class Scene
	{
	public:
		Scene(const std::string& name = "Empty");
		~Scene();

		Entity CreateEntity(const std::string& name = std::string{}, uuid::uuid64 id = uuid::nil_uuid);
		void DestroyEntity(Entity entity);

		void OnUpdateRuntime(Timestep ts);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnViewportResize(uint32_t x, uint32_t y);

		Entity GetPrimaryCameraEntity();

		size_t GetEntityCount() const { return m_registry.size(); }
		Entity GetEntityFromUUID(uuid::uuid64 id) const;
		const EntityMap& GetEntityMap() const { return m_entity_map; }

		void ParentEntity(Entity child, Entity parent);
		void UnparentEntity(Entity child);
	private:
		template <typename T>
		void OnComponentAdded(Entity entity, T& component);
	
	private:
		std::string m_name{ "Empty" };
		entt::registry m_registry;
		EntityMap m_entity_map{};

		uint32_t m_viewport_width = 0, m_viewport_height = 0;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}

#endif
