#ifndef KABLUNK_SCENE_SCENE_H
#define KABLUNK_SCENE_SCENE_H

#include "entt.hpp"


#include "Kablunk/Core/Timestep.h"

namespace Kablunk
{
	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string{});
		void DestroyEntity(Entity entity);

		void OnUpdate(Timestep ts);
		void OnViewportResize(uint32_t x, uint32_t y);
	private:
		template <typename T>
		void OnComponentAdded(Entity entity, T& component);
	
	private:
		entt::registry m_registry;

		uint32_t m_viewport_width = 0, m_viewport_height = 0;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}

#endif
