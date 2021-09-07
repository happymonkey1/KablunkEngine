#include "kablunkpch.h"
#include "Scene.h"

#include "Kablunk/Scene/Components.h"
#include "Kablunk/Renderer/Renderer2D.h"
#include "Kablunk/Renderer/Renderer.h"

#include "Entity.h"

#include <exception>

namespace Kablunk
{
	Scene::Scene()
	{
		// m_registry.on_construct<CameraComponent>().connect<>();
	}

	Scene::~Scene()
	{
		// NativeScript destructor handles cleanup
		/*m_registry.view<NativeScriptComponent>().each(
			[&](auto entity, auto native_script_component)
			{
				if (native_script_component.Instance)
				{
					native_script_component.Instance->OnDestroy();
					native_script_component.DestroyScript(&native_script_component);
				}
			}
		);*/
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
					native_script_component.InstantiateScript();
					native_script_component.Instance->SetEntity({ entity, this });

					try
					{
						native_script_component.Instance->OnAwake();
					}
					catch (std::bad_alloc& e)
					{
						KB_CORE_ERROR("Memery allocation exception '{0}' occurred during OnAwake()", e.what());
						KB_CORE_TRACE("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance.reset();
					}
					catch (std::exception& e)
					{
						KB_CORE_ERROR("Generic exception '{0}' occurred during OnAwake()", e.what());
						KB_CORE_TRACE("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance.reset();
					}
					catch (...)
					{
						KB_CORE_ERROR("Unkown exception occurred during OnAwake()");
						KB_CORE_TRACE("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance.reset();
					}

				}

				if (native_script_component.Instance)
				{
					try
					{
						native_script_component.Instance->OnUpdate(ts);
					}
					catch (std::bad_alloc& e)
					{
						KB_CORE_ERROR("Memery allocation exception '{0}' occurred during OnUpdate()", e.what());
						KB_CORE_WARN("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance.reset();
					}
					catch (std::exception& e)
					{
						KB_CORE_ERROR("Generic exception '{0}' occurred during OnUpdate()", e.what());
						KB_CORE_WARN("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance.reset();
					}
					catch (...)
					{
						KB_CORE_ERROR("Unkown exception occurred during OnUpdate()");
						KB_CORE_WARN("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance.reset();
					}
				}
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
				Renderer2D::DrawSprite({ entity, this });
			}

			Renderer2D::EndScene();
		}
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		m_registry.view<NativeScriptComponent>().each(
			[=](auto entity, auto& native_script_component)
			{
				/*	#TODO
				*	Since there is no concept of creation or destruction of a scene, instead "creation" happens during the update
				*	function if the instance has not been set
				*/
				if (native_script_component.Instance)
				{
					try
					{
						native_script_component.Instance->OnUpdate(ts);
					}
					catch (std::bad_alloc& e)
					{
						KB_CORE_ERROR("Memery allocation exception '{0}' occurred during OnUpdate()", e.what());
						KB_CORE_WARN("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance.reset();
					}
					catch (std::exception& e)
					{
						KB_CORE_ERROR("Generic exception '{0}' occurred during OnUpdate()", e.what());
						KB_CORE_WARN("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance.reset();
					}
					catch (...)
					{
						KB_CORE_ERROR("Unkown exception occurred during OnUpdate()");
						KB_CORE_WARN("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance.reset();
					}
				}
			}
		);

		{
			Renderer2D::BeginScene(camera);

			auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				Renderer2D::DrawSprite({ entity, this });
			}

			Renderer2D::EndScene();
		}

		{
			Renderer::BeginScene(camera);

			auto point_lights = m_registry.view<TransformComponent, PointLightComponent>();
			std::vector<PointLightData> point_lights_data = {};
			uint32_t point_light_count = 0;
			for (auto id : point_lights)
			{
				auto entity = Entity{ id, this };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& plight_comp = entity.GetComponent<PointLightComponent>();

				PointLightData plight_data = {
					transform.Translation,
					plight_comp.Multiplier,
					plight_comp.Radiance,
					plight_comp.Radius,
					plight_comp.Min_radius,
					plight_comp.Falloff
				};

				point_lights_data.push_back(plight_data);
				point_light_count++;
			}
			
			Renderer::SubmitPointLights(point_lights_data, point_light_count);

			auto mesh_group = m_registry.view<TransformComponent, MeshComponent>();
			for (auto entity_id : mesh_group)
			{
				auto entity = Entity{ entity_id, this };
				auto& mesh_comp = entity.GetComponent<MeshComponent>();
				auto& transform = entity.GetComponent<TransformComponent>();
				Renderer::SubmitMesh(mesh_comp.Mesh, transform);
			}

			Renderer::EndScene();
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
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component) { }

	template <>
	void Scene::OnComponentAdded<IdComponent>(Entity entity, IdComponent& component) { }

	template <>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component) { }
	
	template <>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		if (!component.Fixed_aspect_ratio)
			component.Camera.SetViewportSize(m_viewport_width, m_viewport_height);
		KB_CORE_TRACE("CameraComponent Camera initialization code running!");
	}

	template <>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component) { }

	template <>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component) { }

	template <>
	void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component) { }

	template <>
	void Scene::OnComponentAdded<PointLightComponent>(Entity entity, PointLightComponent& component) { }

	template <>
	void Scene::OnComponentAdded<ParentEntityComponent>(Entity entity, ParentEntityComponent& component) { }

	template <>
	void Scene::OnComponentAdded<ChildEntityComponent>(Entity entity, ChildEntityComponent& component) { }
}

