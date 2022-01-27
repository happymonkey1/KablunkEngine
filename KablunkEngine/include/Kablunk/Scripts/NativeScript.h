#ifndef KABLUNK_SCENE_SCRIPTABLE_ENTITY_H
#define KABLUNK_SCENE_SCRIPTABLE_ENTITY_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Scripts/InterfaceIDs.h"
#include "Kablunk/Scene/Entity.h"
#include "Kablunk/Core/Timestep.h"

#include <any>

namespace Kablunk 
{
	// Forward declaration
	struct SpriteRendererComponent;

	class NativeScriptInterface
	{
	public:
		virtual ~NativeScriptInterface() { OnDestroy(); }

		virtual void OnAwake() { }
		virtual void OnUpdate(Timestep ts) { }
		// DEBUG BUILDS ONLY
		virtual void OnImGuiRender() { }

		virtual bool OnRender2D(const SpriteRendererComponent&) { return false; }

		// #TODO virtual function not working
		virtual void OnDestroy() { }
	private:
		friend class Scene;
		friend struct NativeScriptComponent;
	};
	
}


#endif
