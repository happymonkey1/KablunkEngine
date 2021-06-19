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
		// ==========
		//	 Update
		// ==========

		m_registry.view<NativeScriptComponent>().each(
			[=](auto entity, auto& native_script_component)
			{
				/*	#TODO
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
		
		// ==========	
		//	 Render
		// ==========
		Camera*		main_camera{ nullptr };
		glm::mat4*	main_camera_transform{ nullptr };
		auto group = m_registry.group<CameraComponent>(entt::get<TransformComponent>);
		for (auto entity : group)
		{
			const auto& [camera, transform] = group.get<CameraComponent, TransformComponent>(entity);
			
			// Find main camera in scene
			if (camera.Primary)
			{
				main_camera = &camera.Camera;
				main_camera_transform = &transform.Transform;
				break;
			}
		}

		if (main_camera)
		{
			Renderer2D::BeginScene(*main_camera, *main_camera_transform);


			auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				const auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawQuad(transform, sprite.Texture, sprite.Tiling_factor, sprite.Color);
			}

			Renderer2D::EndScene();
		}
	}

}

