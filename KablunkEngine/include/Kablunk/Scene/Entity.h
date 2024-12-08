#ifndef KABLUNK_SCENE_ENTITY_H
#define KABLUNK_SCENE_ENTITY_H

#include <entt.hpp>
#include "Scene.h"

namespace kb
{ // start namespace kb

// Forward declaration
struct TransformComponent;

using entity_handle_t = entt::entity;
constexpr entity_handle_t null_entity = entt::null;

class Entity
{
public:
	Entity() = default;
	Entity(entity_handle_t handle, weak_ptr<Scene> scene);
    ~Entity() noexcept = default;

	Entity(const Entity&) noexcept = default;
    auto operator=(const Entity&) noexcept -> Entity& = default;

	template <typename T>
	bool HasComponent() const;

	template <typename T>
	T& GetComponent();

	template <typename T>
	const T& GetComponent() const;

	template <typename T>
	T& GetOrAddComponent();

	template <typename T, typename... Args>
	T& AddComponent(Args&&... args);

	template <typename T>
	void RemoveComponent();

	TransformComponent& GetTransform();

	bool Valid() const { return m_entity_handle != null_entity && m_scene != nullptr; }

	std::string GetHandleAsString() const { return std::to_string(static_cast<uint64_t>(m_entity_handle)); }
	const entity_handle_t& GetHandle() const { return m_entity_handle; }

	const uuid::uuid64& GetUUID() const;
	const uuid::uuid64& GetParentUUID() const;
	uuid::uuid64 GetSceneUUID() const { return m_scene->GetUUID(); }
	bool HasParent() const;
	Entity GetParent();
	std::vector<uuid::uuid64> GetChildrenCopy() const;
	std::vector<uuid::uuid64>& GetChildren();
	const std::vector<uuid::uuid64>& GetChildren() const;

	bool IsAncestorOf(Entity parent) const;

	bool IsDescendentOf(Entity child) const
	{
		return child.IsAncestorOf(*this);
	}

	void SetParentUUID(const uuid::uuid64& uuid);

	operator uint64_t() const { return static_cast<uint64_t>(m_entity_handle); }
	operator entity_handle_t() const { return m_entity_handle; }

	operator bool() const { return Valid(); }
	bool operator==(const Entity& other) const { return m_entity_handle == other.m_entity_handle && m_scene == other.m_scene; }
	bool operator!=(const Entity& other) const { return !(*this == other); }

private:
	entity_handle_t m_entity_handle;
	weak_ptr<Scene> m_scene;

	friend class CSharpScriptEngine;
	friend class Prefab;
	friend class render::renderer_2d;
};

static_assert(std::is_trivial_v<Entity>);

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
const T& Entity::GetComponent() const
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

} // end namespace kb

#endif
