#include "Panels/SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace Kablunk
{

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_context = context;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		m_context->m_registry.each(
			[&](auto entity_id)
			{
				Entity entity{ entity_id, m_context.get() };
				DrawEntityNode(entity);
				
			}
		);

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) m_selection_context = {};

		ImGui::End();

		ImGui::Begin("Properties");

		if (m_selection_context)
		{
			DrawComponents(m_selection_context);
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity, bool draw_child_node)
	{
		

		// Should probably refactor code so unparented entities do not have a ChildEntityComponent attached
		// Look at entt docs for performance issues
		// Also currently cannot have a child of a child
		if (entity.HasComponent<ChildEntityComponent>() && entity.GetComponent<ChildEntityComponent>().Parent_entity_handle != null_entity) 
			if (!draw_child_node)
				return;

		auto& tag = entity.GetComponent<TagComponent>().Tag;
		ImGuiTreeNodeFlags node_flags = ((m_selection_context == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;

		// Pointer voodo magic
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, node_flags, tag.c_str());

		if (ImGui::IsItemClicked())
			m_selection_context = entity;

		if (opened)
		{
			if (entity.HasComponent<ParentEntityComponent>())
			{
				auto& parent_entity_comp = entity.GetComponent<ParentEntityComponent>();
				for (auto child_handle : parent_entity_comp)
				{
					DrawEntityNode({ child_handle, m_context.get() }, true);
				}
			}
			ImGui::TreePop();
		}
	}

	/*
		#TODO 
		Move to separate file and class
	*/
	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(TagComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Tag"))
			{
				auto& tag = entity.GetComponent<TagComponent>().Tag;

				const uint16_t k_buffer_size = 256;
				char buffer[k_buffer_size];
				// Allocate room for tag + extra buffer
				memset(buffer, 0, k_buffer_size);
				strcpy_s(buffer, k_buffer_size, tag.c_str());
				if (ImGui::InputText("", buffer, k_buffer_size))
				{
					tag = std::string(buffer);
				}

				ImGui::TreePop();
			}
		}

		if (entity.HasComponent<TransformComponent>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
			{

				auto& transform = entity.GetComponent<TransformComponent>();
				ImGui::DragFloat3("Translation", glm::value_ptr(transform.Translation), 0.1f);
				ImGui::DragFloat3("Rotation", glm::value_ptr(transform.Rotation), 1.0f);
				ImGui::DragFloat3("Scale", glm::value_ptr(transform.Scale), 0.1f);
				
				ImGui::TreePop();
			}
		}

		if (entity.HasComponent<CameraComponent>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Camera"))
			{

				auto& camera_component = entity.GetComponent<CameraComponent>();
				auto& camera = camera_component.Camera;

				ImGui::Checkbox("Primary", &camera_component.Primary);

				const char* projection_type_strings[] = { "Perspective", "Orthographic" };
				const char* current_projection_type_str = projection_type_strings[static_cast<int>(camera.GetProjectionType())];

				if (ImGui::BeginCombo("Projection", current_projection_type_str))
				{

					for (int i = 0; i < 2; ++i)
					{
						bool is_selected = current_projection_type_str == projection_type_strings[i];
						if (ImGui::Selectable(projection_type_strings[i], is_selected))
						{
							current_projection_type_str = projection_type_strings[i];
							camera.SetProjectionType(static_cast<SceneCamera::ProjectionType>(i));
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float vertical_fov = glm::degrees(camera.GetPerspectiveVerticalFOV());
					if (ImGui::DragFloat("Size", &vertical_fov, 1.0f, 1.0f, 180.0f))
						camera.SetPerspectiveVerticalFOV(glm::radians(vertical_fov));

					float near_clip = camera.GetPerspectiveNearClip();
					if (ImGui::DragFloat("Near Clip", &near_clip, 0.1f))
						camera.SetPerspectiveNearClip(near_clip);

					float far_clip = camera.GetPerspectiveFarClip();
					if (ImGui::DragFloat("Far Clip", &far_clip, 0.1f))
						camera.SetPerspectiveFarClip(far_clip);
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float ortho_size = camera.GetOrthographicSize();
					if (ImGui::DragFloat("Size", &ortho_size, 0.25f, 1.0f, 10000.0f))
						camera.SetOrthographicSize(ortho_size);

					float near_clip = camera.GetOrthographicNearClip();
					if (ImGui::DragFloat("Near Clip", &near_clip, 0.1f))
						camera.SetOrthographicNearClip(near_clip);

					float far_clip = camera.GetOrthographicFarClip();
					if (ImGui::DragFloat("Far Clip", &far_clip, 0.1f))
						camera.SetOrthographicFarClip(far_clip);

					ImGui::Checkbox("Primary", &camera_component.Fixed_aspect_ratio);
				}

				ImGui::TreePop();
			}
		}

		if (m_display_debug_properties && entity.HasComponent<ParentEntityComponent>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(ParentEntityComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Children"))
			{
				auto& parent_comp = entity.GetComponent<ParentEntityComponent>();
				for (auto child_handle : parent_comp)
				{
					auto child_entity = Entity{ child_handle, m_context.get() };
					auto child_handle_str = child_entity.GetHandleAsString();
					auto child_tag = child_entity.GetComponent<TagComponent>().Tag;
					ImGui::Text("%s (%s)", child_tag.c_str(), child_handle_str.c_str());
				}

				ImGui::TreePop();
			}
		}

		if (m_display_debug_properties && entity.HasComponent<ChildEntityComponent>())
		{
			auto& child_comp = entity.GetComponent<ChildEntityComponent>();
			if (child_comp.HasParent())
			{
				if (ImGui::TreeNodeEx((void*)typeid(ChildEntityComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Parent"))
				{
					auto parent_entity = Entity{ child_comp.GetParent(), m_context.get() };
					auto parent_handle_str = parent_entity.GetHandleAsString();
					auto parent_tag = parent_entity.GetComponent<TagComponent>().Tag;
					ImGui::Text("%s (%s)", parent_tag.c_str(), parent_handle_str.c_str());


					ImGui::TreePop();
				}
			}
		}
	}

}
