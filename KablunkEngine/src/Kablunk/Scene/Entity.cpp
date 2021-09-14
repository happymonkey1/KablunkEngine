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

	bool Entity::HasParent() const
	{
		return GetParentUUID() != uuid::nil_uuid;
	}

	Entity Entity::GetParent()
	{
		if (!HasParent())
			return {};

		return m_scene->GetEntityFromUUID(GetParentUUID());
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
		const auto& children = GetChildren();
		if (children.size() == 0)
			return false;

		for (const auto& child : children)
		{
			if (child == parent.GetUUID())
				return true;
		}

		for (const auto& child : children)
		{
			if (m_scene->GetEntityFromUUID(child).IsAncestorOf(parent))
				return true;
		}

		return false;
	}

	void Entity::SetParentUUID(const uuid::uuid64& uuid)
	{
		auto& parent_comp = GetOrAddComponent<ParentingComponent>();
		parent_comp.Parent = uuid;
	}
}
