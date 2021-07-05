#ifndef KABLUNK_SCENE_SCRIPTABLE_ENTITY_H
#define KABLUNK_SCENE_SCRIPTABLE_ENTITY_H

#include "Kablunk/Scene/Entity.h"

namespace Kablunk
{
	class ScriptableEntity
	{
	public:
		ScriptableEntity() = default;
		
		template <typename T>
		T& GetComponent();
	private:
		Entity m_entity;

		friend class Scene;
	};

	template <typename T>
	T& ScriptableEntity::GetComponent()
	{
		return m_entity.GetComponent<T>();
	}
}

#endif
