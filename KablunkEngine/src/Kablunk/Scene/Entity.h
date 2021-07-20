#ifndef KABLUNK_SCENE_ENTITY_H
#define KABLUNK_SCENE_ENTITY_H

#include <entt.hpp>
#include "Scene.h"

namespace Kablunk
{
	using EntityHandle = entt::entity;
	constexpr EntityHandle null_entity = entt::null;

	class Entity
	{
	public:
		Entity() = default;
		Entity(EntityHandle handle, Scene* scene);

		Entity(const Entity&) = default;

		void AddChild(EntityHandle child);
		void RemoveChild(EntityHandle child);

		// #TODO add something to remove parent ref from ChildCompent is destroyed
		//		 either destroy child entity or unparent.

		template <typename T>
		bool HasComponent() const;

		template <typename T>
		T& GetComponent();

		template <typename T>
		T& GetOrAddComponent();

		template <typename T, typename... Args>
		T& AddComponent(Args&&... args);

		template <typename T>
		void RemoveComponent();

		bool Valid() const { return m_entity_handle != null_entity; }

		std::string GetHandleAsString() const { return std::to_string(static_cast<uint32_t>(m_entity_handle)); }
		const EntityHandle& GetHandle() const { return m_entity_handle; }

		operator uint32_t() const { return static_cast<uint32_t>(m_entity_handle); }
		operator EntityHandle() const { return m_entity_handle; }

		operator bool() const { return Valid(); }
		bool operator==(const Entity& other) const { return m_entity_handle == other.m_entity_handle && m_scene == other.m_scene; }
		bool operator!=(const Entity& other) const { return !(*this == other); }

	private:
		EntityHandle m_entity_handle{ null_entity };
		
		// TODO: replace with weak ref in the future
		Scene* m_scene{ nullptr };
	};

	template <typename T>
	bool Entity::HasComponent() const
	{
		return m_scene->m_registry.all_of<T>(m_entity_handle);
	}

	template <typename T, typename... Args>
	T& Entity::AddComponent(Args&&... args)
	{
		KB_CORE_ASSERT(!HasComponent<T>(), "Trying to add component that already exists!");

		T& component = m_scene->m_registry.emplace<T>(m_entity_handle, std::forward<Args>(args)...);
		m_scene->OnComponentAdded<T>(*this, component);
		return component;
	}

	template <typename T>
	T& Entity::GetComponent()
	{
		KB_CORE_ASSERT(HasComponent<T>(), "Component does not exist on entity!");
		return m_scene->m_registry.get<T>(m_entity_handle);
	}

	template <typename T>
	T& Entity::GetOrAddComponent()
	{
		if (HasComponent<T>()) 
			return GetComponent<T>();
		else
			return AddComponent<T>();
	}

	template <typename T>
	void Entity::RemoveComponent()
	{
		KB_CORE_ASSERT(HasComponent<T>(), "Trying to remove component that does not exist!");
		m_scene->m_registry.remove<T>(m_entity_handle);
	}
}

#endif
