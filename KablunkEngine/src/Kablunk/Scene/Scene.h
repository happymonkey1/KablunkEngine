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

		void OnUpdate(Timestep ts);
	private:
		entt::registry m_registry;

		friend class Entity;
	};
}

#endif