#include "kablunkpch.h"
#include "Entity.h"

#include "Components.h"

namespace Kablunk
{
	Entity::Entity(EntityHandle handle, Scene* scene)
		: m_entity_handle{ handle }, m_scene{ scene }
	{

	}

	const uuid::uuid64& Entity::GetUUID() const
	{
		if (HasComponent<IdComponent>())
			return GetComponent<IdComponent>().Id;
		else
			return uuid::nil_uuid;
	}

	const uuid::uuid64& Entity::GetParentUUID() const
	{
		return GetComponent<ParentingComponent>().Parent;
	}

	std::vector<uuid::uuid64>& Entity::GetChildren()
	{
		return GetComponent<ParentingComponent>().Children;
	}

	const std::vector<uuid::uuid64>& Entity::GetChildren() const
	{
		return GetComponent<ParentingComponent>().Children;
	}

	bool Entity::IsAncestorOf(Entity parent) const
	{
		// DFS
		bool is_ancestor = false;
		if (parent.GetUUID() == GetParentUUID()) 
			return true;

		is_ancestor = IsAncestorOf(m_scene->GetEntityFromUUID(parent.GetParentUUID()));

		return is_ancestor;
	}

	void Entity::SetParentUUID(const uuid::uuid64& uuid)
	{
		auto& parent_comp = GetOrAddComponent<ParentingComponent>();
		parent_comp.Parent = uuid;
	}


	void Entity::AddChild(EntityHandle child_handle)
	{
#if 0
		if (!HasComponent<ParentEntityComponent>()) AddComponent<ParentEntityComponent>();
		auto& parent_comp = GetComponent<ParentEntityComponent>();
		KB_CORE_ASSERT(!parent_comp.ContainsHandle(child_handle), "Trying to add child that already exists!");
		parent_comp.AddChildHandle(child_handle);

		auto child = Entity{ child_handle, m_scene };
		if (child.HasComponent<ChildEntityComponent>())
		{
			// We can use this scene because parented entities MUST be in the same scene
			auto& child_component = child.GetComponent<ChildEntityComponent>();
			auto other_parent = Entity{ child_component.GetParent(), m_scene };
			other_parent.RemoveChild(m_entity_handle);
		}
		else
			child.AddComponent<ChildEntityComponent>();

		child.GetComponent<ChildEntityComponent>().SetParent(m_entity_handle);
#endif
	}

	void Entity::RemoveChild(EntityHandle child_handle)
	{
#if 0
		KB_CORE_ASSERT(!HasComponent<ParentEntityComponent>(), "Trying to remove child from an entity that doesn't have parent component!");
		auto parent_entity_comp = GetComponent<ParentEntityComponent>();
		KB_CORE_ASSERT(parent_entity_comp.ContainsHandle(child_handle), "Trying to remove child that doesn't exist!");
		parent_entity_comp.RemoveChildHandle(child_handle);

		auto child = Entity{ child_handle, m_scene };
		KB_CORE_ASSERT(!HasComponent<ChildEntityComponent>(), "Trying to remove parent reference from child that does not have a child component!");
		child.GetComponent<ChildEntityComponent>().SetParent(null_entity);
#endif
	}
}
