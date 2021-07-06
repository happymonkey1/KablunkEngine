#include "kablunkpch.h"
#include "Scene.h"

#include "Kablunk/Scene/Components.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include "Entity.h"

namespace Kablunk
{
	Scene::Scene()
	{

	}

	Scene::~Scene()
	{
		m_registry.view<NativeScriptComponent>().each(
			[&](auto entity, auto native_script_component)
			{
				if (native_script_component.Instance)
				{
					native_script_component.Instance->OnDestroy();
					native_script_component.DestroyScript(&native_script_component);
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
					native_script_component.Instance = native_script_component.InstantiateScript();
					native_script_component.Instance->m_entity = Entity{ entity, this };
					native_script_component.Instance->OnCreate();
				}

				native_script_component.Instance->OnUpdate(ts);
			}
		);
		
		// ==========	
		//	 Render
		// ==========

		Camera*		main_camera{ nullptr };
		glm::mat4	main_camera_transform;
		auto view = m_registry.view<CameraComponent, TransformComponent>();
		for (auto entity : view)
		{
			auto [camera, transform] = view.get<CameraComponent, TransformComponent>(entity);
			
			// Find main camera in scene
			if (camera.Primary)
			{
				main_camera = &camera.Camera;
				main_camera_transform = transform.GetTransform();
				break;
			}
		}

		if (main_camera)
		{
			Renderer2D::BeginScene(*main_camera, main_camera_transform);

			auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawQuad(transform, sprite.Texture, sprite.Tiling_factor, sprite.Color);
			}

			Renderer2D::EndScene();
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_viewport_width = width; 
		m_viewport_height = height;

		// resize camera(s) which do not have fixed aspect ratios
		auto view = m_registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& camera_component = view.get<CameraComponent>(entity);
			if (!camera_component.Fixed_aspect_ratio)
				camera_component.Camera.SetViewportSize(width, height);
			
		}
	}
}

