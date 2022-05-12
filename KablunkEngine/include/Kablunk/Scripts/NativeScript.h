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

		virtual void OnEvent(Event& e) { };
		// DEBUG BUILDS ONLY
		virtual void OnImGuiRender() { }

		virtual bool OnRender2D(const SpriteRendererComponent&) { return false; }

		virtual void OnDestroy() { }

		void Destroy();
	protected:
		template <typename T>
		bool HasComponent() const { return m_entity.HasComponent<T>(); }

		template <typename T>
		T& GetComponent() { return m_entity.GetComponent<T>(); }

		template <typename T>
		T& GetComponent() const { return m_entity.GetComponent<T>() }
	private:
		void BindEntity(Entity entity) { m_entity = entity; }
	private:
		Entity m_entity = {};

		friend class Scene;
		friend struct NativeScriptComponent;
	};
	
}


#endif
