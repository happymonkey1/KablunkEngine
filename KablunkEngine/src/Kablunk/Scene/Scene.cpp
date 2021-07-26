#include "kablunkpch.h"
#include "Scene.h"

#include "Kablunk/Scene/Components.h"
#include "Kablunk/Renderer/Renderer2D.h"

#include "Entity.h"

namespace Kablunk
{
	Scene::Scene()
	{
		// m_registry.on_construct<CameraComponent>().connect<>();
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

	Entity Scene::CreateEntity(const std::string& name, uuid::uuid64 id)
	{
		Entity entity = { m_registry.create(), this };
		entity.AddComponent<TransformComponent>();
		
		// Check if we should use uuid passed into when creating the id component
		if (uuid::is_nil(id))
			entity.AddComponent<IdComponent>();
		else
			entity.AddComponent<IdComponent>(id);

		auto& tag = entity.AddComponent<TagComponent>(name);
		tag = name.empty() ? "Blank Entity" : name;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_registry.destroy(entity);
	}

	void Scene::OnUpdateRuntime(Timestep ts)
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
				Renderer2D::DrawQuad({ entity, this });
			}

			Renderer2D::EndScene();
		}
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);

		auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group)
		{
			Renderer2D::DrawQuad({ entity, this });
		}

		Renderer2D::EndScene();
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

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return { entity, this };
		}

		return {};
	}

	// #WARNING Templated code SHOULD be written in the header file, BUT since we are declaring specializations,
	//			the compiler will not complain. However, this means that every new component added needs to have
	//			the template specialization added. seems bad and a waste of time. 

	// #TODO	Long story short, figure out a better way to do this.

	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		KB_CORE_ASSERT(false, "No default OnComponentAdded!");
	}

	template <>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{

	}

	template <>
	void Scene::OnComponentAdded<IdComponent>(Entity entity, IdComponent& component)
	{

	}

	template <>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{

	}
	
	template <>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		if (!component.Fixed_aspect_ratio)
			component.Camera.SetViewportSize(m_viewport_width, m_viewport_height);
		KB_CORE_TRACE("CameraComponent Camera initialization code running!");
	}

	template <>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
		
	}

	template <>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{

	}

	template <>
	void Scene::OnComponentAdded<ParentEntityComponent>(Entity entity, ParentEntityComponent& component)
	{

	}

	template <>
	void Scene::OnComponentAdded<ChildEntityComponent>(Entity entity, ChildEntityComponent& component)
	{

	}
}

