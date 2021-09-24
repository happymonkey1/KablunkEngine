#include "Panels/SceneHierarchyPanel.h"

#include <Kablunk/Imgui/ImGuiWrappers.h>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui_internal.h"

#include <filesystem>

namespace Kablunk
{
	// #TODO bad!
	extern const std::filesystem::path g_asset_path;
	extern const std::filesystem::path g_resources_path;

	std::string KablunkRigidBodyTypeToString(RigidBody2DComponent::RigidBodyType type)
	{
		switch (type)
		{
		case RigidBody2DComponent::RigidBodyType::Static:		return std::string{ "Static" };
		case RigidBody2DComponent::RigidBodyType::Dynamic:		return std::string{ "Dynamic" };
		case RigidBody2DComponent::RigidBodyType::Kinematic:	return std::string{ "Kinematic" };
		
		}
		KB_CORE_ASSERT(false, "unknown body type"); 
		return std::string{ "Static" };
	}

	RigidBody2DComponent::RigidBodyType StringToKablunkRigidBodyType(const std::string& type)
	{
		if (type == "Static")
			return RigidBody2DComponent::RigidBodyType::Static;
		else if (type == "Dynamic")
			return RigidBody2DComponent::RigidBodyType::Dynamic;
		else if (type == "Kinematic")			
			return RigidBody2DComponent::RigidBodyType::Kinematic;
		else
		{
			KB_CORE_ASSERT(false, "unknown body type");
			return RigidBody2DComponent::RigidBodyType::Static;
		}
	}

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

	void SceneHierarchyPanel::UI_RenameScenePopup()
	{
		if (m_rename_scene_popup_open)
			ImGui::OpenPopup("Rename Scene");

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Rename Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
		{
			const float button_size = 24.0f;
			//const float button_pos_x = ImGui::GetWindowWidth() - button_size - ImGui::GetCurrentWindow()->WindowPadding.x / 2.0f;
			// #TODO button on right side
			if (ImGui::Button("X", { button_size, button_size }))
				m_rename_scene_popup_open = false;

			char buffer[256];
			strcpy_s<256>(buffer, m_context->m_name.c_str());

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
			ImGui::Text("New name");

			ImGui::SameLine();
			//ImGui::PushItemWidth(-1);

			// #FIXME window flashing when typing
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3.0f);
			if (ImGui::InputText("##rename", buffer, 256))
			{
				m_context->m_name = buffer;
			}

			if (!m_rename_scene_popup_open)
				ImGui::CloseCurrentPopup();

			//ImGui::PopItemWidth();

			// For some reason the imgui wrappers aren't resizing the window correctly
#if 0
			UI::BeginProperties();

			auto scene_name = m_context->m_name;
			UI::Property("Name", scene_name);

			UI::EndProperties();
#endif

			ImGui::EndPopup();
		}
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		auto window_rect = ImRect{ ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax() };

		// Scene tree node
		auto scene_tree_node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
		if (m_context->GetEntityCount() == 0)
			scene_tree_node_flags |= ImGuiTreeNodeFlags_Leaf;

		UI_RenameScenePopup();

		if (ImGui::TreeNodeEx(m_context->m_name.c_str(), scene_tree_node_flags))
		{
			if (UI::IsMouseDownOnItem(1))
				m_rename_scene_popup_open = true;

			auto group = m_context->m_registry.view<IdComponent, ParentingComponent>();
			for (auto id : group)
			{
				Entity entity{ id, m_context.get() };
				if (entity.GetParentUUID() == uuid::nil_uuid)
					UI_DrawEntityNode(entity);
			}

			ImGui::TreePop();
		}

		if (ImGui::BeginDragDropTargetCustom(window_rect, ImGui::GetCurrentWindow()->ID))
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_entity_hierarchy");
			if (payload)
			{
				Entity& dropped_entity = *(Entity*)payload->Data;
				m_context->UnparentEntity(dropped_entity);
			}

			ImGui::EndDragDropTarget();
		}
		

		if (UI::IsMouseDownOnDockedWindow()) m_selection_context = {};

		// Context menu for right-clicking blank space
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("Create Blank Entity"))
			{
				auto entity = m_context->CreateEntity();
				m_selection_context = entity;
			}
			
			if (ImGui::MenuItem("Create Sprite"))
			{
				auto entity = m_context->CreateEntity("Blank Sprite");
				entity.AddComponent<SpriteRendererComponent>();
				m_selection_context = entity;
			}

			if (ImGui::MenuItem("Create Camera"))
			{
				auto entity = m_context->CreateEntity("Camera");
				bool is_first_camera = m_context->GetPrimaryCameraEntity() == null_entity ? true : false;
				auto& camera_comp = entity.AddComponent<CameraComponent>();
				camera_comp.Primary = is_first_camera;
				m_selection_context = entity;
			}

			if (ImGui::BeginMenu("3D"))
			{
				if (ImGui::MenuItem("Create Cube"))
				{
					auto entity = m_context->CreateEntity("Cube");
					auto& mesh_comp = entity.AddComponent<MeshComponent>(MeshFactory::CreateCube(1.0f, entity));
					m_selection_context = entity;
				}

				if (ImGui::MenuItem("Create Point Light"))
				{
					auto entity = m_context->CreateEntity("Point Light");
					auto& plight_comp = entity.AddComponent<PointLightComponent>(1.0f, glm::vec3{ 1.0f }, 10.0f, 1.0f, 1.0f);
					m_selection_context = entity;
				}
				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		

		ImGui::End();

		ImGui::Begin("Properties");

		if (m_selection_context)
			UI_DrawComponents(m_selection_context);

		ImGui::End();
	}

	void SceneHierarchyPanel::UI_DrawEntityNode(Entity entity, bool draw_child_node)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		ImGuiTreeNodeFlags node_flags = ((m_selection_context == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
		node_flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		

		if (entity.GetChildren().empty())
			node_flags |= ImGuiTreeNodeFlags_Leaf;

		// Pointer voodo magic
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity, node_flags, tag.c_str());

		if (ImGui::IsItemClicked())
			m_selection_context = entity;

		bool entity_deleted = false;

		// Context menu for right-clicking entity in hierarchy
		if (ImGui::BeginPopupContextItem(0, 1))
		{
			if (ImGui::MenuItem("Duplicate"))
				m_context->DuplicateEntity(entity);

			if (ImGui::MenuItem("Delete Entity"))
				entity_deleted = true;
			

			ImGui::EndPopup();
		}


		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			ImGui::Text(entity.GetComponent<TagComponent>().Tag.c_str());
			ImGui::SetDragDropPayload("scene_entity_hierarchy", &entity, sizeof(entity));
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_entity_hierarchy");
			if (payload)
			{
				Entity& dropped_entity = *(Entity*)payload->Data;
				m_context->ParentEntity(dropped_entity, entity);
			}

			ImGui::EndDragDropTarget();
		}

		if (opened)
		{
			for (const auto& child_id : entity.GetChildren())
			{
				Entity child_entity = m_context->GetEntityFromUUID(child_id);
				if (child_entity)
					UI_DrawEntityNode(child_entity);
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
		UI::ShiftCursorY(3.0f);
		ImGui::Text(label.c_str());
		UI::ShiftCursorY(-3.0f);
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
	void SceneHierarchyPanel::UI_DrawComponents(Entity entity)
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

			if (!m_selection_context.HasComponent<RigidBody2DComponent>() && ImGui::MenuItem("RigidBody 2D"))
			{
				m_selection_context.AddComponent<RigidBody2DComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (!m_selection_context.HasComponent<BoxCollider2DComponent>() && ImGui::MenuItem("Box Collider 2D"))
			{
				// Box collider needs rigidbody
				if (!m_selection_context.HasComponent<RigidBody2DComponent>())
					m_selection_context.AddComponent<RigidBody2DComponent>();

				m_selection_context.AddComponent<BoxCollider2DComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (!m_selection_context.HasComponent<CircleCollider2DComponent>() && ImGui::MenuItem("Circle Collider 2D"))
			{
				// Box collider needs rigidbody
				if (!m_selection_context.HasComponent<RigidBody2DComponent>())
					m_selection_context.AddComponent<RigidBody2DComponent>();

				m_selection_context.AddComponent<CircleCollider2DComponent>();
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
				UI::BeginProperties();

				UI::Property("Primary", &component.Primary);

				const char* projection_type_strings[] = { "Perspective", "Orthographic" };

				UI::PropertyDropdown("Projection Type", projection_type_strings, 2, camera.GetProjectionType());
				
				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float fov = glm::degrees(camera.GetPerspectiveVerticalFOV());
					if (UI::Property("FOV", fov, 1.0f, 1.0f, 200.0f))
						camera.SetPerspectiveVerticalFOV(glm::radians(fov));
					UI::Property("Near Clip", camera.GetPerspectiveNearClip(), 0.1f, 0.001f, 10000.0f);
					UI::Property("Far Clip", camera.GetPerspectiveFarClip(), 0.1f, 0.001f, 10000.0f);
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					UI::Property("Size", camera.GetOrthographicSize(), 1.0f, 1.0f, 200.0f);
					UI::Property("Near Clip", camera.GetOrthographicNearClip(), 0.1f, 0.001f, 10000.0f);
					UI::Property("Far Clip", camera.GetOrthographicFarClip(), 0.1f, 0.001f, 10000.0f);
				}

				UI::EndProperties();
			});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [this](auto& component)
			{

				UI::BeginProperties();

				UI::PropertyColorEdit4("Tint Color", component.Color);

				if (UI::PropertyImageButton("Texture", component.Texture, {32, 32}, {0.0f, 1.0f}, {1.0f, 0.0f}))
				{
					auto filepath = FileDialog::OpenFile("Image File (*.png)\0*.png\0");
					if (!filepath.empty())
					{
						component.Texture = Asset<Texture2D>(filepath);
					}
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const auto payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const auto path_wchar_str = (const wchar_t*)payload->Data;
						auto path = std::filesystem::path{ g_asset_path / path_wchar_str };
						auto path_str = path.string();
						if (path.extension() == ".png")
							component.Texture = Asset<Texture2D>(path_str);
						else
							KB_CORE_ERROR("Tried to load non image file as a texture, {0}", path_str);
					}
					ImGui::EndDragDropTarget();
				}

#if 0
				if (m_display_debug_properties)
				{

					UI::PushItemDisabled();
					UI::Property("", uuid::to_string(sprite_texture_asset.GetUUID()));
					UI::PopItemDisabled();
				}
#endif

				UI::Property("Tiling Factor", component.Tiling_factor);

				UI::EndProperties();
			});

		DrawComponent<NativeScriptComponent>("Native Script", entity, [&](auto& component)
			{
				UI::BeginProperties();
				if (component.Instance)
				{
					std::vector<std::string> name_vec = Parser::CPP::FindStructNames(component.GetFilepath());
					if (name_vec.empty())
						KB_CORE_ASSERT(false, "Script loaded somehow but could not find name for component!");

					UI::PropertyReadOnlyString("Script Name", name_vec[0]);

					// #TODO relative path based on project
					UI::PropertyReadOnlyString("Filepath", component.GetFilepath());
				}
				else
				{
					if (UI::Button("Add"))
					{
						auto filepath = FileDialog::OpenFile("Header File (*.h)\0*.h\0");
						if (!filepath.empty())
						{
							// #FIXME relative path when projects are implemented
							//std::filesystem::path relative_path = std::filesystem::relative(filepath, g_asset_path);
							component.BindEditor(filepath, entity);
						}
					}
				}
				UI::EndProperties();
			});

		DrawComponent<MeshComponent>("Mesh", entity, [&](auto& component)
			{
				const char* add_or_change_button_text;
				if (!component.Mesh)
					add_or_change_button_text = "Add";
				else
					add_or_change_button_text = "Replace";

				if (ImGui::Button(add_or_change_button_text))
				{
					auto filepath = FileDialog::OpenFile("Mesh (*.fbx)\0*.fbx\0");
					if (!filepath.empty())
						component.LoadMeshFromFileEditor(filepath, entity);
				}

				ImGui::Text("File:");
				ImGui::PushStyleColor(ImGuiCol_Text, { 0.494f, 0.494f, 0.494f, 1.0f });
				ImGui::TextWrapped(component.Filepath.c_str());
				ImGui::PopStyleColor();
			});

		DrawComponent<PointLightComponent>("Point Light", entity, [&](auto& component)
			{
				UI::BeginProperties();

				UI::Property("Multiplier", component.Multiplier);
				UI::PropertyColorEdit3("Radiance", component.Radiance);
				UI::Property("Radius", component.Radius);
				UI::Property("Minimum Radius", component.Min_radius);
				UI::Property("Falloff Distance", component.Falloff);

				UI::EndProperties();
			});

		DrawComponent<RigidBody2DComponent>("RigidBody 2D", entity, [&](RigidBody2DComponent& component)
			{
				UI::BeginProperties();


				std::vector<std::string> options = { "Static", "Dynamic", "Kinematic" };
				int32_t index = std::find(options.begin(), options.end(), KablunkRigidBodyTypeToString(component.Type)) - options.begin();
				if (UI::PropertyDropdown("Type", options, options.size(), &index))
					component.Type = StringToKablunkRigidBodyType(options[index]);
				
				UI::Property("Fixed Rotation", &component.Fixed_rotation);

				UI::EndProperties();
			});

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [&](BoxCollider2DComponent& component)
			{
				UI::BeginProperties();

				UI::Property("Offset", component.Offset);

				UI::Property("Density", component.Density);
				UI::Property("Friction", component.Friction, 0.01f, 0.0f, 1.0f);
				UI::Property("Restitution", component.Restitution, 0.01f, 0.0f, 1.0f);
				UI::Property("Restitution Threshold", component.Restitution_threshold);

				UI::EndProperties();
			});

		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [&](CircleCollider2DComponent& component)
			{
				UI::BeginProperties();

				UI::Property("Offset", component.Offset);

				UI::Property("Density", component.Density);
				UI::Property("Friction", component.Friction, 0.01f, 0.0f, 1.0f);
				UI::Property("Restitution", component.Restitution, 0.01f, 0.0f, 1.0f);
				UI::Property("Restitution Threshold", component.Restitution_threshold);

				UI::EndProperties();
			});

		// Debug Panels
		if (m_display_debug_properties)
		{
#if 0
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
#endif
		}

	}

}
