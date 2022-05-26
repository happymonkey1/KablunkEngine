#ifndef KABLUNK_SCENE_SCENE_H
#define KABLUNK_SCENE_SCENE_H

#include "entt.hpp"

#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Core/Timestep.h"
#include "Kablunk/Core/Uuid64.h"
#include "Kablunk/Renderer/EditorCamera.h"
//#include "Kablunk/Renderer/SceneRenderer.h"

class b2World;

namespace Kablunk
{
	class Entity;
	class SceneRenderer;
	
	using EntityMap = std::unordered_map<uuid::uuid64, Entity>;
	constexpr const char* DEFAULT_SCENE_NAME = "Untitled Scene";

	struct PointLight
	{
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		float Multiplier = { 1.0f };
		glm::vec3 Radiance = { 1.0f, 1.0f, 1.0f };
		float Radius = { 10.0f };
		float Min_radius = { 1.0f };
		float Falloff = { 1.0f };

		char Padding[8]{}; // Don't know why this is needed, but sizeof PointLight = 48 bytes makes it work???
	};

	struct LightEnvironmentData
	{
		// #TODO Directional Lights

		std::vector<PointLight> point_lights;
		size_t GetPointLightsSize() const { return point_lights.size() * sizeof(PointLight); }
	};

	class Scene : public RefCounted
	{
	public:
		Scene(const std::string& name = DEFAULT_SCENE_NAME);
		~Scene();

		static IntrusiveRef<Scene> Copy(IntrusiveRef<Scene> src_scene);
		static WeakRef<Scene> GetScene(uuid::uuid64 scene_id);

		Entity CreateEntity(const std::string& name = "", uuid::uuid64 id = uuid::nil_uuid);
		void DestroyEntity(Entity entity);

		void OnEvent(Event& e);

		void OnStartRuntime();
		void OnStopRuntime();

		void OnUpdateRuntime(Timestep ts);
		void OnRenderRuntime(IntrusiveRef<SceneRenderer> scene_renderer, EditorCamera* camera = nullptr);
		void OnEventRuntime(Event& e);
		void OnUpdateEditor(Timestep ts);
		void OnRenderEditor(IntrusiveRef<SceneRenderer> scene_renderer, EditorCamera& camera);
		void OnEventEditor(Event& e);
		void OnViewportResize(uint32_t x, uint32_t y);

		void OnImGuiRender();

		Entity GetPrimaryCameraEntity();

		size_t GetEntityCount() const { return m_registry.size(); }
		Entity GetEntityFromUUID(uuid::uuid64 id) const;
		const EntityMap& GetEntityMap() const { return m_entity_map; }

		uuid::uuid64 GetUUID() const { return m_scene_id; }

		Entity DuplicateEntity(Entity entity);

		void ParentEntity(Entity child, Entity parent);
		void UnparentEntity(Entity child);

		template <typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_registry.view<Components...>();
		}

		const std::string& GetSceneName() const { return m_name; }
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

		LightEnvironmentData m_light_environment;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
		friend class SceneRenderer;

		// fixed update
		float last_time = 0.0f;
		const float FIXED_TIMESTEP = 0.5f; // #TODO expose to project settings
	};
}

#endif
