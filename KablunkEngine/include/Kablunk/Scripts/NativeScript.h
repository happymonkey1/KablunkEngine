#ifndef KABLUNK_SCENE_SCRIPTABLE_ENTITY_H
#define KABLUNK_SCENE_SCRIPTABLE_ENTITY_H

#include "Kablunk/Scripts/InterfaceIDs.h"
#include "Kablunk/Scene/Entity.h"
#include "Kablunk/Core/Timestep.h"
#include "RCCPP/RuntimeObjectSystem/IObject.h"


#include <any>

namespace Kablunk 
{
	class NativeScript : public IObject
	{
	public:
		NativeScript();
		virtual ~NativeScript()
		{
			OnDestroy();
		}
		
		template <typename T>
		T& GetComponent()
		{
			return m_entity.GetComponent<T>();
		}

		// #TODO fix api and remove this
		void SetEntity(Entity entity)
		{
			m_entity = entity;
		}

		virtual void OnAwake() { }
		virtual void OnUpdate(Timestep ts) { }
		// #TODO virtual function not working
		virtual void OnDestroy() { }
	private:
		Entity m_entity{ };

		friend class Scene;
		friend struct NativeScriptComponent;
	};
	
}

#endif
