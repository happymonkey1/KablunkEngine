#ifndef KABLUNK_SCENE_SCRIPTABLE_ENTITY_H
#define KABLUNK_SCENE_SCRIPTABLE_ENTITY_H

#include "Kablunk/Scene/Entity.h"

namespace Kablunk
{
	class ScriptableEntity
	{
	public:
		virtual ~ScriptableEntity() = default;
		
		template <typename T>
		T& GetComponent()
		{
			return m_entity.GetComponent<T>();
		}

	protected:
		virtual void OnCreate()				{ }
		virtual void OnUpdate(Timestep ts)	{ }
		virtual void OnDestroy()			{ }

	private:
		Entity m_entity;

		friend class Scene;
	};
	
}

#endif
