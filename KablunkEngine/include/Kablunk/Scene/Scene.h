#ifndef KABLUNK_SCENE_SCENE_H
#define KABLUNK_SCENE_SCENE_H

#include "entt.hpp"

#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Core/Timestep.h"
#include "Kablunk/Core/Uuid64.h"
#include "Kablunk/Renderer/EditorCamera.h"

class b2World;

namespace Kablunk
{
	class Entity;

	
	using EntityMap = std::unordered_map<uuid::uuid64, Entity>;
	constexpr const char* DEFAULT_SCENE_NAME = "Untitled Scene";

	class Scene
	{
	public:
		Scene(const std::string& name = DEFAULT_SCENE_NAME);
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> src_scene);
		static WeakRef<Scene> GetScene(uuid::uuid64 scene_id);

		Entity CreateEntity(const std::string& name = "", uuid::uuid64 id = uuid::nil_uuid);
		void DestroyEntity(Entity entity);

		void OnStartRuntime();
		void OnStopRuntime();

		void OnUpdateRuntime(Timestep ts);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnViewportResize(uint32_t x, uint32_t y);

		Entity GetPrimaryCameraEntity();

		size_t GetEntityCount() const { return m_registry.size(); }
		Entity GetEntityFromUUID(uuid::uuid64 id) const;
		const EntityMap& GetEntityMap() const { return m_entity_map; }

		uuid::uuid64 GetUUID() const { return m_scene_id; }

		Entity DuplicateEntity(Entity entity);

		void ParentEntity(Entity child, Entity parent);
		void UnparentEntity(Entity child);

		template <typename T, typename... Args>
		auto GetAllEntitiesWith()
		{
			return m_registry->view<T, Args...>();
		}
	private:
		template <typename T>
		void OnComponentAdded(Entity entity, T& component);
	
		void OnCSharpScriptComponentConstruct(entt::registry& registry, entt::entity entity);
		void OnCSharpScriptComponentDestroy(entt::registry& registry, entt::entity entity);
	private:
		std::string m_name{ DEFAULT_SCENE_NAME };
		uuid::uuid64 m_scene_id = uuid::generate();
		entt::registry m_registry;
		EntityMap m_entity_map{};

		uint32_t m_viewport_width = 0, m_viewport_height = 0;

		b2World* m_box2D_world = nullptr;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}

#endif
