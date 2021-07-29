#ifndef KABLUNK_SCENE_SCRIPTABLE_ENTITY_H
#define KABLUNK_SCENE_SCRIPTABLE_ENTITY_H

#include "Kablunk/Scene/Entity.h"
#include "Kablunk/Core/Timestep.h"

#include <any>

namespace Kablunk
{
	class NativeScript
	{
	public:
		NativeScript();
		NativeScript(Entity entity);
		virtual ~NativeScript() = default;
		
		template <typename T>
		T& GetComponent()
		{
			return m_entity.GetComponent<T>();
		}

		// #TODO fix api and remove this
		void SetEntity(Entity entity)
		{
			KB_CORE_WARN("NativeScript: FIX THE DAMN API!!!");
			m_entity = entity;
		}

	protected:
		virtual void OnAwake() { }
		virtual void OnUpdate(Timestep ts)  { }
	private:
		Entity m_entity;

		friend class Scene;
	};
	
}

#endif