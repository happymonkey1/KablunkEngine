#include "Panels/SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui_internal.h"

#include <filesystem>

namespace Kablunk
{
	// #TODO bad!
	extern const std::filesystem::path g_asset_path;

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
		
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_context = context;
		m_selection_context = {};
		KB_CORE_WARN("Selection context cleared!");
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

		// Context menu for right-clicking blank space
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("Create Blank Entity"))
			{
				m_context->CreateEntity();
			}
			
			if (ImGui::MenuItem("Create Sprite"))
			{
				auto entity = m_context->CreateEntity("Blank Sprite");
				entity.AddComponent<SpriteRendererComponent>();
			}

			if (ImGui::MenuItem("Create Camera"))
			{
				auto entity = m_context->CreateEntity("Camera");
				bool is_first_camera = m_context->GetPrimaryCameraEntity() == null_entity ? true : false;
				auto& camera_comp = entity.AddComponent<CameraComponent>();
				camera_comp.Primary = is_first_camera;
			}

			if (ImGui::MenuItem("Create Cube"))
			{
				auto entity = m_context->CreateEntity("Cube");
				auto& mesh_comp = entity.AddComponent<MeshComponent>(MeshFactory::CreateCube(1.0f));
			}

			if (ImGui::MenuItem("Create Point Light"))
			{
				auto entity = m_context->CreateEntity("Point Light");
				auto& plight_comp = entity.AddComponent<PointLightComponent>(1.0f, glm::vec3{ 1.0f }, 10.0f, 1.0f, 1.0f);
			}

			ImGui::EndPopup();
		}

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
		

		// Should probably refactor code so un-parented entities do not have a ChildEntityComponent attached
		// Look at entt docs for performance issues
		// Also currently cannot have a child of a child
		if (entity.HasComponent<ChildEntityComponent>() && entity.GetComponent<ChildEntityComponent>().Parent_entity_handle != null_entity) 
			if (!draw_child_node)
				return;

		auto& tag = entity.GetComponent<TagComponent>().Tag;
		ImGuiTreeNodeFlags node_flags = ((m_selection_context == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		node_flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		// Pointer voodo magic
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity, node_flags, tag.c_str());

		if (ImGui::IsItemClicked())
			m_selection_context = entity;


		bool entity_deleted = false;
		// Context menu for right-clicking entity in hierarchy
		if (ImGui::BeginPopupContextItem(0, 1))
		{
			if (ImGui::MenuItem("Delete Entity"))
				entity_deleted = true;
			

			ImGui::EndPopup();
		}


		if (opened)
		{
			// Parenting placeholder code
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

		// Deferred deletion so ui and other stuff can still happen
		if (entity_deleted)
		{
			m_context->DestroyEntity(entity);

			if (m_selection_context == entity) m_selection_context = { };
		}
	}

	// #TODO move elsewhere

	static bool DrawVec3Control(const std::string& label, glm::vec3& values, float reset_value = 0.0f, float value_tuning = 0.1f, float column_width = 100.0f)
	{
		// #TODO check values before clamping format to 2 decimal places.

		ImGuiIO& io = ImGui::GetIO();
		auto bold_font = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2, 0, false);

		ImGui::SetColumnWidth(0, column_width);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

		float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		auto button_size = ImVec2{ line_height + 3.0f, line_height };
		bool updated = false;

		// Red
		ImGui::PushStyleColor(ImGuiCol_Button,			{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,	{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,	{ 0.8f, 0.1f, 0.15f, 1.0f });
		
		ImGui::PushFont(bold_font);
		if (ImGui::Button("X", button_size))
		{
			values.x = reset_value;
			updated = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);


		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &values.x, value_tuning, 0.0f, 0.0f, "%.2f")) updated = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		// Green
		ImGui::PushStyleColor(ImGuiCol_Button,			{ 0.2f, 0.7f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   { 0.3f, 0.8f, 0.4f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,    { 0.2f, 0.7f, 0.3f, 1.0f });

		ImGui::PushFont(bold_font);
		if (ImGui::Button("Y", button_size))
		{
			values.y = reset_value;
			updated = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &values.y, value_tuning, 0.0f, 0.0f, "%.2f")) updated = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		// Blue
		ImGui::PushStyleColor(ImGuiCol_Button,			{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,	{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,	{ 0.1f, 0.25f, 0.8f, 1.0f });

		ImGui::PushFont(bold_font);
		if (ImGui::Button("Z", button_size))
		{
			values.z = reset_value;
			updated = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &values.z, value_tuning, 0.0f, 0.0f, "%.2f")) updated = true;
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();

		return updated;
	}

	template <typename ComponentType, typename UIFunction>
	static void DrawComponent(const std::string& label, Entity entity, UIFunction UIDrawFunction)
	{
		const ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap 
			| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;

		if (entity.HasComponent<ComponentType>())
		{
			auto& component = entity.GetComponent<ComponentType>();

			// Get content region before adding treenode
			auto content_region_available = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 4 });

			float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(ComponentType).hash_code(), tree_node_flags, label.c_str());
			
			ImGui::PopStyleVar();
			ImGui::SameLine(content_region_available.x - line_height * 0.5f);
			
		//	const float button_padding_left = 50.0f;
			if (ImGui::Button("...", { line_height, line_height }))	ImGui::OpenPopup("##ComponentSettings");

			bool remove_component = false;
			if (ImGui::BeginPopup("##ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove Component"))
					remove_component = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				UIDrawFunction(component);
				ImGui::TreePop();
			}

			if (remove_component)
			{
				entity.RemoveComponent<ComponentType>();
			}
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
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			const uint16_t k_buffer_size = 256;
			char buffer[k_buffer_size];
			// Allocate room for tag + extra buffer
			memset(buffer, 0, k_buffer_size);
			strcpy_s(buffer, k_buffer_size, tag.c_str());
			if (ImGui::InputText("##Tag", buffer, k_buffer_size))
			{
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("##AddComponent");

		if (ImGui::BeginPopup("##AddComponent"))
		{
			if (!m_selection_context.HasComponent<CameraComponent>() && ImGui::MenuItem("Camera"))
			{
				m_selection_context.AddComponent<CameraComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (!m_selection_context.HasComponent<SpriteRendererComponent>() && ImGui::MenuItem("Sprite Renderer"))
			{
				m_selection_context.AddComponent<SpriteRendererComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (!m_selection_context.HasComponent<NativeScriptComponent>() && ImGui::MenuItem("Native Script"))
			{
				m_selection_context.AddComponent<NativeScriptComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (!m_selection_context.HasComponent<MeshComponent>() && ImGui::MenuItem("Mesh"))
			{
				m_selection_context.AddComponent<MeshComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (!m_selection_context.HasComponent<PointLightComponent>() && ImGui::MenuItem("Point Light"))
			{
				m_selection_context.AddComponent<PointLightComponent>();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		if (m_display_debug_properties)
		{
			if (entity.HasComponent<IdComponent>())
			{
				auto& id = entity.GetComponent<IdComponent>().Id;

				// Way to big of a buffer
				constexpr uint16_t k_buffer_size = 256;
				char buffer[k_buffer_size];

				memset(buffer, 0, k_buffer_size);
				strcpy_s(buffer, k_buffer_size, uuid::to_string(id).c_str());
				ImGui::TextColored({ 0.494f, 0.494f, 0.494f, 1.0f }, buffer);
			}
		}

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
			{
				DrawVec3Control("Translation", component.Translation);

				auto rotation = glm::degrees(component.Rotation);
				if (DrawVec3Control("Rotation", rotation, 0.0f, 1.0f))
					component.Rotation = glm::radians(rotation);

				DrawVec3Control("Scale", component.Scale, 1.0f);
			});

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
			{
				auto& camera = component.Camera;

				ImGui::Checkbox("Primary", &component.Primary);

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
					if (ImGui::DragFloat("FOV", &vertical_fov, 1.0f, 1.0f, 200.0f))
						camera.SetPerspectiveVerticalFOV(glm::radians(vertical_fov));

					float near_clip = camera.GetPerspectiveNearClip();
					if (ImGui::DragFloat("Near Clip", &near_clip, 0.1f, 0.001f, 10000.0f))
						camera.SetPerspectiveNearClip(near_clip);

					float far_clip = camera.GetPerspectiveFarClip();
					if (ImGui::DragFloat("Far Clip", &far_clip, 0.1f, 0.001f, 10000.0f))
						camera.SetPerspectiveFarClip(far_clip);
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float ortho_size = camera.GetOrthographicSize();
					if (ImGui::DragFloat("Size", &ortho_size, 0.25f, 0.1f, 0.0f))
						camera.SetOrthographicSize(ortho_size);

					float near_clip = camera.GetOrthographicNearClip();
					if (ImGui::DragFloat("Near Clip", &near_clip, 0.1f))
						camera.SetOrthographicNearClip(near_clip);

					float far_clip = camera.GetOrthographicFarClip();
					if (ImGui::DragFloat("Far Clip", &far_clip, 0.1f))
						camera.SetOrthographicFarClip(far_clip);
				}
			});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [this](auto& component)
			{
				ImGui::ColorEdit4("Tint Color", glm::value_ptr(component.Color));

				auto& sprite_texture_asset = component.Texture;
				auto sprite_texture_renderer_id = sprite_texture_asset.Get()->GetRendererID();
				if (ImGui::ImageButton(reinterpret_cast<void*>(static_cast<uint64_t>(sprite_texture_renderer_id)), { 32, 32 }, { 0.0f, 1.0f }, { 1.0f, 0.0f }))
				{
					// #TODO change texture dynamically
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const auto payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const auto path_wchar_str = (const wchar_t*)payload->Data;
						auto path = std::filesystem::path{ g_asset_path / path_wchar_str };
						auto path_str = path.string();
						if (path.extension() == ".png")
							sprite_texture_asset = Asset<Texture2D>(path_str);
						else
							KB_CORE_ERROR("Tried to load non kablunkscene file as scene, {0}", path_str);
					}
					ImGui::EndDragDropTarget();
				}

				if (m_display_debug_properties)
				{
					ImGui::SameLine();

					// Way too big of a buffer
					constexpr uint16_t k_buffer_size = 256;
					char buffer[k_buffer_size];

					memset(buffer, 0, k_buffer_size);
					strcpy_s(buffer, k_buffer_size, uuid::to_string(sprite_texture_asset.GetUUID()).c_str());
					ImGui::TextColored({ 0.494f, 0.494f, 0.494f, 1.0f }, buffer);
				}

				

				float tiling_factor = component.Tiling_factor;
				if (ImGui::DragFloat("Tiling Factor", &tiling_factor, 0.1f))
					component.Tiling_factor = tiling_factor;
			});

		DrawComponent<NativeScriptComponent>("Native Script", entity, [&](auto& component)
			{
				if (component.Instance)
				{
					ImGui::Text("Script loaded");
					ImGui::Text("Filepath:");
					
					ImGui::PushStyleColor(ImGuiCol_Text, { 0.494f, 0.494f, 0.494f, 1.0f });
					ImGui::TextWrapped(component.Filepath.c_str());
					ImGui::PopStyleColor();
				}
				else
				{
					if (ImGui::Button("Add"))
					{
						auto filepath = FileDialog::OpenFile("Header File (*.h)\0*.h\0");
						if (!filepath.empty())
						{
							component.BindEditor(filepath, entity);
						}
					}
				}
			});

		DrawComponent<MeshComponent>("Mesh", entity, [&](auto& component)
			{
				ImGui::Text("FIXME");
			});

		DrawComponent<PointLightComponent>("Point Light", entity, [&](auto& component)
			{
				float multiplier = component.Multiplier;
				if (ImGui::DragFloat("Multiplier", &multiplier, 0.1f))
					component.Multiplier = multiplier;

				glm::vec3 radiance = component.Radiance;
				if (ImGui::ColorEdit3("Radiance", glm::value_ptr(radiance)))
					component.Radiance = radiance;

				float r = component.Radius;
				if (ImGui::DragFloat("Radius", &r, 0.1f))
					component.Radius = r;

				float min_r = component.Min_radius;
				if (ImGui::DragFloat("Minimum Radius", &min_r, 0.1f))
					component.Min_radius = min_r;

				float falloff = component.Falloff;
				if (ImGui::DragFloat("Falloff Distance", &falloff, 0.1f))
					component.Falloff = falloff;
			});

		// Debug Panels
		if (m_display_debug_properties)
		{
			DrawComponent<ParentEntityComponent>("Children", entity, [&](auto& component)
				{
					for (auto child_handle : component)
					{
						auto child_entity = Entity{ child_handle, m_context.get() };
						auto child_handle_str = child_entity.GetHandleAsString();
						auto child_tag = child_entity.GetComponent<TagComponent>().Tag;
						ImGui::Text("%s (%s)", child_tag.c_str(), child_handle_str.c_str());
					}
				});

			DrawComponent<ChildEntityComponent>("Parent", entity, [&](auto& component)
				{
					auto parent_entity = Entity{ component.GetParent(), m_context.get() };
					auto parent_handle_str = parent_entity.GetHandleAsString();
					auto parent_tag = parent_entity.GetComponent<TagComponent>().Tag;
					ImGui::Text("%s (%s)", parent_tag.c_str(), parent_handle_str.c_str());
				});
		}

	}

}
