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

	class INativeScript
	{
	public:
		virtual ~INativeScript() { };

		virtual void OnAwake() = 0;
		virtual void OnUpdate(Timestep ts) = 0;
		virtual void OnDestroy() = 0;

		virtual void OnEvent(Event& e) = 0;
		virtual void OnImGuiRender() = 0;
		virtual bool OnRender2D(const SpriteRendererComponent&) = 0;

	protected:
		virtual void bind_entity(Entity entity) = 0;
	private:
		friend class Scene;
	};

	class NativeScript : public INativeScript
	{
	public:
		virtual ~NativeScript() override { OnDestroy(); }

		virtual void OnAwake() override { }
		virtual void OnUpdate(Timestep ts) override { }

		virtual void OnEvent(Event& e) override { };
		// DEBUG BUILDS ONLY
		virtual void OnImGuiRender() override { }

		virtual bool OnRender2D(const SpriteRendererComponent&) override { return false; }

		virtual void OnDestroy() { }

		void Destroy();
	protected:
		template <typename T>
		bool HasComponent() const { return m_entity.HasComponent<T>(); }

		template <typename T>
		T& GetComponent() { return m_entity.GetComponent<T>(); }

		template <typename T>
		const T& GetComponent() const { return m_entity.GetComponent<T>(); }
	private:
		virtual void bind_entity(Entity entity) override { m_entity = entity; }
	private:
		Entity m_entity = {};

		friend class Scene;
		friend struct NativeScriptComponent;
	};
	
}


#endif
