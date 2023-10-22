#ifndef KABLUNK_SCENE_SCENE_H
#define KABLUNK_SCENE_SCENE_H

#include "entt.hpp"

#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Core/Timestep.h"
#include "Kablunk/Core/Uuid64.h"
#include "Kablunk/Renderer/EditorCamera.h"


//#include "Kablunk/Renderer/SceneRenderer.h"

class b2World;

namespace kb
{
	class Entity;
	class SceneRenderer;
	struct TransformComponent;

    class Renderer2D;
	
	using EntityMap = kb::unordered_flat_map<uuid::uuid64, Entity>;
	constexpr const char* DEFAULT_SCENE_NAME = "Untitled Scene";

	struct PointLight
	{
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		float Multiplier = { 1.0f };
		glm::vec3 Radiance = { 1.0f, 1.0f, 1.0f };
		float Radius = { 10.0f };
		float Min_radius = { 1.0f };
		float Falloff = { 1.0f };

		char Padding[8]{}; 
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

		static ref<Scene> Create();
		static ref<Scene> Copy(ref<Scene> src_scene);
		static WeakRef<Scene> GetScene(uuid::uuid64 scene_id);

		Entity CreateEntity(const std::string& name = "", uuid::uuid64 id = uuid::nil_uuid);
		void DestroyEntity(Entity entity);

		void OnEvent(Event& e);

		void OnStartRuntime();
		void OnStopRuntime();

		void OnUpdateRuntime(Timestep ts);
		void OnRenderRuntime(ref<SceneRenderer> scene_renderer, ref<Renderer2D> p_renderer_2d, EditorCamera* camera = nullptr);
		void OnEventRuntime(Event& e);
		void OnUpdateEditor(Timestep ts);
		void OnRenderEditor(ref<SceneRenderer> scene_renderer, ref<Renderer2D> p_renderer_2d, EditorCamera& camera);
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

		template <typename ComponentT>
		bool CopyComponentIfItExists(entt::entity dst, entt::entity src, entt::registry& dest_reg)
		{
			if (m_registry.any_of<ComponentT>(src))
			{
				auto& src_comp = m_registry.get<ComponentT>(src);
				dest_reg.emplace_or_replace<ComponentT>(dst, src_comp);
				return true;
			}
			else
				return false;
		}

		const std::string& GetSceneName() const { return m_name; }

		glm::mat4 get_world_space_transform_matrix(Entity entity) const;
		TransformComponent get_world_space_transform(Entity entity) const;
	private:
		template <typename T>
		void OnComponentAdded(Entity entity, T& component);
	
		void OnCSharpScriptComponentConstruct(entt::registry& registry, entt::entity entity);
		void OnCSharpScriptComponentDestroy(entt::registry& registry, entt::entity entity);

		void on_native_script_component_construct(entt::registry& registry, entt::entity entity);
		void on_native_script_component_destroy(entt::registry& registry, entt::entity entity);

		// callback when CameraComponent is created in registry
		void on_camera_component_construct(entt::registry& registry, entt::entity entity);
		// callback when CameraComponent is removed from registry
		void on_camera_component_destroy(entt::registry& registry, entt::entity entity);
	private:
		std::string m_name{ DEFAULT_SCENE_NAME };
		uuid::uuid64 m_scene_id = uuid::generate();
		entt::registry m_registry;
		EntityMap m_entity_map{};

		uint32_t m_viewport_width = 0, m_viewport_height = 0;

		b2World* m_box2D_world = nullptr;

		LightEnvironmentData m_light_environment;

		// reference to primary camera entity. set when a CameraComponent is created that is tagged with primary, 
		// or when the reference is null and GetPrimaryCameraEntity is called for the first time.
		Entity* m_primary_camera_entity = nullptr;

		// fixed update
		float last_time = 0.0f;
		const float FIXED_TIMESTEP = 0.5f; // #TODO expose to project settings

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
		friend class SceneRenderer;
		friend class Prefab;
	};
}

#endif
