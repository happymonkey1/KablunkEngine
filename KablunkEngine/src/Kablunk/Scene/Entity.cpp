#include "kablunkpch.h"
#include "Entity.h"

#include "Components.h"

namespace Kablunk
{
	Entity::Entity(EntityHandle handle, Scene* scene)
		: m_entity_handle{ handle }, m_scene{ scene }
	{

	}

	void Entity::AddChild(EntityHandle child_handle)
	{
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
	}

	void Entity::RemoveChild(EntityHandle child_handle)
	{
		KB_CORE_ASSERT(!HasComponent<ParentEntityComponent>(), "Trying to remove child from an entity that doesn't have parent component!");
		auto parent_entity_comp = GetComponent<ParentEntityComponent>();
		KB_CORE_ASSERT(parent_entity_comp.ContainsHandle(child_handle), "Trying to remove child that doesn't exist!");
		parent_entity_comp.RemoveChildHandle(child_handle);

		auto child = Entity{ child_handle, m_scene };
		KB_CORE_ASSERT(!HasComponent<ChildEntityComponent>(), "Trying to remove parent reference from child that does not have a child component!");
		child.GetComponent<ChildEntityComponent>().SetParent(null_entity);
	}
}
