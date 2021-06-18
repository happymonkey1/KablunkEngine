#include "kablunkpch.h"
#include "Scene.h"

#include "Kablunk/Scene/Components.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include "Entity.h"

namespace Kablunk
{
	Scene::Scene()
	{

		entt::entity e = m_registry.create();

		m_registry.emplace<TransformComponent>(e);


	}

	Scene::~Scene()
	{
		m_registry.view<NativeScriptComponent>().each(
			[&](auto entity, auto native_script_component)
			{
				if (native_script_component.Instance)
				{
					native_script_component.OnDestroyFunction();
					native_script_component.DestroyFunction();
				}
			}
		);
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>(name);
		tag = name.empty() ? "Blank Entity" : name;

		return entity;
	}

	void Scene::OnUpdate(Timestep ts)
	{
		// Update
		m_registry.view<NativeScriptComponent>().each(
			[&](auto entity, NativeScriptComponent native_script_component)
			{
				/* #TODO
				*	Since there is no concept of creation or destruction of a scene, instead "creation" happens during the update
				*	function if the instance has not been set
				*/ 
				if (!native_script_component.Instance)
				{
					native_script_component.InstantiateFunction();
					native_script_component.Instance->m_entity = Entity{ entity, this };
					native_script_component.OnCreateFunction();
				}

				native_script_component.OnUpdateFunction(ts);
			}
		);
			
		// Render
		auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group)
		{
			const auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
			
			Renderer2D::DrawQuad(transform, sprite.Texture, sprite.Tiling_factor, sprite.Color);
		}
	}

}

