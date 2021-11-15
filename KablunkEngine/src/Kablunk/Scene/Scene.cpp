#include "kablunkpch.h"
#include "Kablunk/Scene/Scene.h"

#include "Kablunk/Scene/Components.h"
#include "Kablunk/Renderer/Renderer2D.h"
#include "Kablunk/Renderer/Renderer.h"
#include "Kablunk/Scene/Entity.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_fixture.h>

#include <exception>

namespace Kablunk
{
	Scene::Scene(const std::string& name)
		: m_name{ name }
	{
		
	}

	Scene::~Scene()
	{
		
	}

	b2BodyType KablunkRigidBody2DToBox2DType(RigidBody2DComponent::RigidBodyType type)
	{
		switch (type)
		{
		case RigidBody2DComponent::RigidBodyType::Static:		return b2BodyType::b2_staticBody;
		case RigidBody2DComponent::RigidBodyType::Dynamic:		return b2BodyType::b2_dynamicBody;
		case RigidBody2DComponent::RigidBodyType::Kinematic:	return b2BodyType::b2_kinematicBody;
		default:												KB_CORE_ASSERT(false, "unknown body type"); return b2_staticBody;
		}
	}

	template <typename ComponentT>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const EntityMap& entity_map)
	{
		auto view = src.view<ComponentT>();
		for (auto e : view)
		{
			uuid::uuid64 UUID = src.get<IdComponent>(e).Id;
			entt::entity dst_entt_id = entity_map.at(UUID);

			auto& component = src.get<ComponentT>(e);
			dst.emplace_or_replace<ComponentT>(dst_entt_id, component);
		}
	}

	template <typename ComponentT>
	static bool CopyComponentIfItExists(entt::entity dst, entt::entity src, entt::registry& reg)
	{
		if (reg.any_of<ComponentT>(src))
		{
			auto& src_comp = reg.get<ComponentT>(src);
			reg.emplace_or_replace<ComponentT>(dst, src_comp);
			return true;
		}
		else
			return false;
	}

	template <typename ComponentT>
	static bool CopyComponentIfItExists(entt::entity dst, entt::entity src, entt::registry& src_reg, entt::registry& dest_reg)
	{
		if (src_reg.any_of<ComponentT>(src))
		{
			auto& src_comp = src_reg.get<ComponentT>(src);
			dest_reg.emplace_or_replace<ComponentT>(dst, src_comp);
			return true;
		}
		else
			return false;
	}

	Ref<Scene> Scene::Copy(Ref<Scene> src_scene)
	{
		Ref<Scene> dest_scene = CreateRef<Scene>();
		dest_scene->m_name = src_scene->m_name;

		dest_scene->m_entity_map = src_scene->m_entity_map;

		dest_scene->m_viewport_width = src_scene->m_viewport_width;
		dest_scene->m_viewport_height = src_scene->m_viewport_height;

		auto& src_scene_reg = src_scene->m_registry;
		auto& dest_scene_reg = dest_scene->m_registry;
		auto id_view = src_scene_reg.view<IdComponent>();
		for (auto id : id_view)
		{
			uuid::uuid64 uuid = src_scene_reg.get<IdComponent>(id).Id;
			const auto& tag = src_scene_reg.get<TagComponent>(id).Tag;

			dest_scene->CreateEntity(tag, uuid);
		}

		CopyComponent<TransformComponent>(dest_scene_reg, src_scene_reg, dest_scene->m_entity_map);
		CopyComponent<SpriteRendererComponent>(dest_scene_reg, src_scene_reg, dest_scene->m_entity_map);
		CopyComponent<CircleRendererComponent>(dest_scene_reg, src_scene_reg, dest_scene->m_entity_map);
		CopyComponent<CameraComponent>(dest_scene_reg, src_scene_reg, dest_scene->m_entity_map);
		CopyComponent<NativeScriptComponent>(dest_scene_reg, src_scene_reg, dest_scene->m_entity_map);
		CopyComponent<MeshComponent>(dest_scene_reg, src_scene_reg, dest_scene->m_entity_map);
		CopyComponent<PointLightComponent>(dest_scene_reg, src_scene_reg, dest_scene->m_entity_map);
		CopyComponent<RigidBody2DComponent>(dest_scene_reg, src_scene_reg, dest_scene->m_entity_map);
		CopyComponent<BoxCollider2DComponent>(dest_scene_reg, src_scene_reg, dest_scene->m_entity_map);
		CopyComponent<CircleCollider2DComponent>(dest_scene_reg, src_scene_reg, dest_scene->m_entity_map);

		// Bind all native script components
		auto view = dest_scene_reg.view<NativeScriptComponent>();
		for (auto e : view)
		{
			Entity entity{ e, dest_scene.get() };
			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			nsc.BindEditor(entity);
		}

		// #TODO copy parenting components

		return dest_scene;
	}

	Entity Scene::CreateEntity(const std::string& name, uuid::uuid64 id)
	{
		Entity entity = { m_registry.create(), this };
		entity.AddComponent<TransformComponent>();
		
		// Check if we should use uuid passed into when creating the id component
		uuid::uuid64 entity_uuid;
		if (uuid::is_nil(id))
			entity_uuid = entity.AddComponent<IdComponent>().Id;
		else
		{
			entity_uuid = id;
			entity.AddComponent<IdComponent>(id);
		}

		TagComponent& tag_comp = entity.AddComponent<TagComponent>();
		tag_comp.Tag = name.empty() ? "Blank Entity" : name;

		entity.AddComponent<ParentingComponent>();

		m_entity_map.insert({ entity_uuid, entity });

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_registry.destroy(entity);
	}

	void Scene::OnStartRuntime()
	{
		// #TODO expose gravity in editor panel
		m_box2D_world = new b2World{ { 0.0f, -9.8f } };

		auto view = m_registry.view<RigidBody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetTransform();
			auto& rb2d_comp = entity.GetComponent<RigidBody2DComponent>();

			b2BodyDef body_def;
			body_def.type = KablunkRigidBody2DToBox2DType(rb2d_comp.Type);
			body_def.position.Set(transform.Translation.x, transform.Translation.y);
			body_def.angle = transform.Rotation.z;

			b2Body* body = m_box2D_world->CreateBody(&body_def);
			body->SetFixedRotation(rb2d_comp.Fixed_rotation);
			rb2d_comp.Runtime_body = body;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d_comp = entity.GetComponent<BoxCollider2DComponent>();
				// #FIXME offset isn't working
				body_def.position += b2Vec2{ bc2d_comp.Offset.x, bc2d_comp.Offset.y };

				b2PolygonShape polygon_shape;
				polygon_shape.SetAsBox(bc2d_comp.Size.x * transform.Scale.x, bc2d_comp.Size.y * transform.Scale.y);

				b2FixtureDef fixture_def;
				fixture_def.shape = &polygon_shape;
				fixture_def.density = bc2d_comp.Density;
				fixture_def.friction = bc2d_comp.Friction;
				fixture_def.restitution = bc2d_comp.Restitution;
				fixture_def.restitutionThreshold = bc2d_comp.Restitution_threshold;
				body->CreateFixture(&fixture_def);
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d_comp = entity.GetComponent<CircleCollider2DComponent>();
				// #FIXME offset isn't working
				body_def.position += b2Vec2{ cc2d_comp.Offset.x, cc2d_comp.Offset.y };

				b2CircleShape circle;
				circle.m_radius = cc2d_comp.Radius * transform.Scale.x;

				b2FixtureDef fixture_def;
				fixture_def.shape = &circle;
				fixture_def.density = cc2d_comp.Density;
				fixture_def.friction = cc2d_comp.Friction;
				fixture_def.restitution = cc2d_comp.Restitution;
				fixture_def.restitutionThreshold = cc2d_comp.Restitution_threshold;
				body->CreateFixture(&fixture_def);
			}
		}
	}

	void Scene::OnStopRuntime()
	{
		delete m_box2D_world;
		m_box2D_world = nullptr;
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
					(*native_script_component.Instance)->SetEntity({ entity, this });

					try
					{
						(*native_script_component.Instance)->OnAwake();
					}
					catch (std::bad_alloc& e)
					{
						KB_CORE_ERROR("Memery allocation exception '{0}' occurred during OnAwake()", e.what());
						KB_CORE_TRACE("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance = nullptr;
					}
					catch (std::exception& e)
					{
						KB_CORE_ERROR("Generic exception '{0}' occurred during OnAwake()", e.what());
						KB_CORE_TRACE("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance = nullptr;
					}
					catch (...)
					{
						KB_CORE_ERROR("Unkown exception occurred during OnAwake()");
						KB_CORE_TRACE("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance = nullptr;
					}

				}

				if (native_script_component.Instance)
				{
					try
					{
						(*native_script_component.Instance)->OnUpdate(ts);
					}
					catch (std::bad_alloc& e)
					{
						KB_CORE_ERROR("Memery allocation exception '{0}' occurred during OnUpdate()", e.what());
						KB_CORE_WARN("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance = nullptr;
					}
					catch (std::exception& e)
					{
						KB_CORE_ERROR("Generic exception '{0}' occurred during OnUpdate()", e.what());
						KB_CORE_WARN("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance = nullptr;
					}
					catch (...)
					{
						KB_CORE_ERROR("Unkown exception occurred during OnUpdate()");
						KB_CORE_WARN("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance = nullptr;
					}
				}
			}
		);
		
		// ===========	
		//	 Physics
		// ===========

		{
			// #TODO expose to editor
			const uint32_t velocityIterations = 6;
			const uint32_t positionIterations = 2;

			m_box2D_world->Step(ts, velocityIterations, positionIterations);

			// update transforms from box2D data
			auto view = m_registry.view<RigidBody2DComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				auto& transform = entity.GetTransform();
				auto& rb2d_comp = entity.GetComponent<RigidBody2DComponent>();

				b2Body* body = static_cast<b2Body*>(rb2d_comp.Runtime_body);
				const auto& pos = body->GetPosition();
				transform.Translation.x = pos.x;
				transform.Translation.y = pos.y;
				transform.Rotation.z = body->GetAngle();
			}
		}

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

		{
			if (main_camera)
			{
				Renderer::BeginScene(*main_camera, main_camera_transform);

				auto point_lights = m_registry.view<TransformComponent, PointLightComponent>();
				std::vector<PointLight> point_lights_data = {};
				uint32_t point_light_count = 0;
				for (auto id : point_lights)
				{
					auto entity = Entity{ id, this };
					auto& transform = entity.GetComponent<TransformComponent>();
					auto& plight_comp = entity.GetComponent<PointLightComponent>();

					PointLight plight_data = {
						transform.Translation, //{ transform.Translation.x, transform.Translation.y, transform.Translation.z },
						plight_comp.Multiplier,
						plight_comp.Radiance, //{ plight_comp.Radiance.x, plight_comp.Radiance.y, plight_comp.Radiance.z },
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
					if (mesh_comp.Mesh)
						Renderer::SubmitMesh(mesh_comp.Mesh, transform);
				}

				Renderer::EndScene();
			}
		}

		if (main_camera)
		{
			Renderer2D::BeginScene(*main_camera, main_camera_transform);
			
			{
				auto view = m_registry.view<TransformComponent, SpriteRendererComponent>();
				for (auto entity : view)
					Renderer2D::DrawSprite({ entity, this });
			}
			
			{
				auto view = m_registry.view<TransformComponent, CircleRendererComponent>();
				for (auto entity : view)
				{
					auto& [transform, circle_component] = view.get<TransformComponent, CircleRendererComponent>(entity);
					Renderer2D::DrawCircle(transform.GetTransform(), circle_component.Color, circle_component.Radius, circle_component.Thickness, circle_component.Fade, (int32_t)entity);
				}
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
						if (!NativeScriptEngine::AreScriptsCompiling())
							(*native_script_component.Instance)->OnUpdate(ts);
					}
					catch (std::bad_alloc& e)
					{
						KB_CORE_ERROR("Memery allocation exception '{0}' occurred during OnUpdate()", e.what());
						KB_CORE_WARN("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance = nullptr;
					}
					catch (std::exception& e)
					{
						KB_CORE_ERROR("Generic exception '{0}' occurred during OnUpdate()", e.what());
						KB_CORE_WARN("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance = nullptr;
					}
					catch (...)
					{
						KB_CORE_ERROR("Unkown exception occurred during OnUpdate()");
						KB_CORE_WARN("Script '{0}' failed! Unloading!", native_script_component.Filepath);
						native_script_component.Instance = nullptr;
					}
				}
			}
		);

		Renderer2D::BeginScene(camera);
		{
			auto view = m_registry.view<TransformComponent, SpriteRendererComponent>();
			for (auto entity : view)
				Renderer2D::DrawSprite({ entity, this });
		}

		{
			auto view = m_registry.view<TransformComponent, CircleRendererComponent>();
			for (auto entity : view)
			{
				auto& [transform, circle_component] = view.get<TransformComponent, CircleRendererComponent>(entity);
				Renderer2D::DrawCircle(transform.GetTransform(), circle_component.Color, circle_component.Radius, circle_component.Thickness, circle_component.Fade, (int32_t)entity);
			}
		}
		Renderer2D::EndScene();

		{
			Renderer::BeginScene(camera);

			auto point_lights = m_registry.view<TransformComponent, PointLightComponent>();
			std::vector<PointLight> point_lights_data = {};
			uint32_t point_light_count = 0;
			for (auto id : point_lights)
			{
				auto entity = Entity{ id, this };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& plight_comp = entity.GetComponent<PointLightComponent>();

				PointLight plight_data = {
					transform.Translation, //{ transform.Translation.x, transform.Translation.y, transform.Translation.z },
					plight_comp.Multiplier,
					plight_comp.Radiance, //{ plight_comp.Radiance.x, plight_comp.Radiance.y, plight_comp.Radiance.z },
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
				if (mesh_comp.Mesh)
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

	Entity Scene::GetEntityFromUUID(uuid::uuid64 id) const
	{

		auto it = m_entity_map.find(id);
		return it != m_entity_map.end() ? it->second : Entity{};
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		Entity new_entity;
		if (entity.HasComponent<TagComponent>())
		{
			std::string tag = entity.GetComponent<TagComponent>().Tag;
			new_entity = CreateEntity(tag);
		}
		else
		{
			KB_CORE_ERROR("Duplicating entity which doesn't have a tag component!");
			new_entity = CreateEntity();
		}
		
		CopyComponentIfItExists<TransformComponent>(new_entity.GetHandle(), entity.GetHandle(), m_registry);
		CopyComponentIfItExists<SpriteRendererComponent>(new_entity.GetHandle(), entity.GetHandle(), m_registry);
		CopyComponentIfItExists<CircleRendererComponent>(new_entity.GetHandle(), entity.GetHandle(), m_registry);
		CopyComponentIfItExists<CameraComponent>(new_entity.GetHandle(), entity.GetHandle(), m_registry);
		if (CopyComponentIfItExists<NativeScriptComponent>(new_entity.GetHandle(), entity.GetHandle(), m_registry))
			new_entity.GetComponent<NativeScriptComponent>().BindEditor(new_entity);
		CopyComponentIfItExists<MeshComponent>(new_entity.GetHandle(), entity.GetHandle(), m_registry);
		CopyComponentIfItExists<PointLightComponent>(new_entity.GetHandle(), entity.GetHandle(), m_registry);
		CopyComponentIfItExists<RigidBody2DComponent>(new_entity.GetHandle(), entity.GetHandle(), m_registry);
		CopyComponentIfItExists<BoxCollider2DComponent>(new_entity.GetHandle(), entity.GetHandle(), m_registry);
		CopyComponentIfItExists<CircleCollider2DComponent>(new_entity.GetHandle(), entity.GetHandle(), m_registry);

		// Need to copy ParentingComponent manually 
		auto children = entity.GetChildrenCopy();
		for (auto child_id : children)
		{
			Entity child = GetEntityFromUUID(child_id);
			KB_CORE_ASSERT(child, "Child entity is null. Scene probably corrupted!");
			if (child)
			{
				Entity new_child = DuplicateEntity(child);

				UnparentEntity(new_child);

				new_child.SetParentUUID(new_entity.GetUUID());
				new_entity.GetChildren().push_back(new_child.GetUUID());
			}
		}

		if (entity.HasParent())
		{
			Entity parent = GetEntityFromUUID(entity.GetParentUUID());
			KB_CORE_ASSERT(parent, "Failed to find parent entity. Scene probably corrup!");
			new_entity.SetParentUUID(entity.GetParentUUID());
			parent.GetChildren().push_back(new_entity.GetUUID());
		}

		return new_entity;
	}

	void Scene::ParentEntity(Entity child, Entity parent)
	{
		if (parent.IsDescendentOf(child))
		{
			UnparentEntity(parent);

			Entity new_parent = child.GetParent();
			if (new_parent)
			{
				UnparentEntity(child);
				ParentEntity(parent, new_parent);
			}
		}
		else
		{
			Entity other_parent = child.GetParent();
			if (other_parent)
				UnparentEntity(child);
		}

		parent.GetChildren().push_back(child.GetUUID());
		child.SetParentUUID(parent.GetUUID());
	}

	void Scene::UnparentEntity(Entity child)
	{
		Entity parent = GetEntityFromUUID(child.GetParentUUID());
		if (!parent)
			return;

		auto& children = parent.GetChildren();
		children.erase(std::find(children.begin(), children.end(), child.GetUUID()));

		child.SetParentUUID(uuid::nil_uuid);
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
	void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component) { }

	template <>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component) { }

	template <>
	void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component) { }

	template <>
	void Scene::OnComponentAdded<PointLightComponent>(Entity entity, PointLightComponent& component) { }

	template <>
	void Scene::OnComponentAdded<ParentingComponent>(Entity entity, ParentingComponent& component) { }

	template <>
	void Scene::OnComponentAdded<RigidBody2DComponent>(Entity entity, RigidBody2DComponent& component) { }

	template <>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component) { }

	template <>
	void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component) { }

}

