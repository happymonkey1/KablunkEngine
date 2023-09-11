#include "Panels/SceneHierarchyPanel.h"

#include <Kablunk/Project/ProjectManager.h>
#include <Kablunk/Scripts/CSharpScriptEngine.h>
#include <Kablunk/Imgui/ImGuiWrappers.h>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui_internal.h"

// #TODO refactor application singleton and remove
#include "Kablunk/Core/Application.h"

#include <filesystem>

#define DISABLE_NATIVE_SCRIPT 0

namespace Kablunk
{

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

	SceneHierarchyPanel::SceneHierarchyPanel(const IntrusiveRef<Scene>& context)
	{
		SetContext(context);

	}

	void SceneHierarchyPanel::SetContext(const IntrusiveRef<Scene>& context)
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

	template <typename ComponentT>
	void DrawMaterialTable(IntrusiveRef<MaterialTable> mesh_material_table)
	{
		if (UI::BeginTreeNode("Materials"))
		{

			for (size_t i = 0; i < mesh_material_table->GetMaterialCount(); i++)
			{
				UI::BeginProperties();

				if (i == mesh_material_table->GetMaterialCount())
					ImGui::Separator();


				std::string label = fmt::format("[Material {0}]", i);

				std::string id = fmt::format("{0}-{1}", label, i);
				ImGui::PushID(id.c_str());

				IntrusiveRef<MaterialAsset> mesh_material_asset = mesh_material_table->GetMaterial(i);
				std::string mesh_material_name = mesh_material_asset->GetMaterial()->GetName();
				if (mesh_material_name.empty())
					mesh_material_name = "Unnamed Material";

				UI::PushItemDisabled();
				UI::Property("Name", mesh_material_name);
				UI::PopItemDisabled();

				if (render::get_render_pipeline() == RendererPipelineDescriptor::PHONG_DIFFUSE)
				{
					// #TODO check if this needs to be set on render thread
					float& ambient_strength = mesh_material_asset->GetMaterial()->GetFloat("u_MaterialUniforms.AmbientStrength");
					if (UI::Property("Ambient Strength", ambient_strength, 0.01f, 0.0f, 1.0f))
						mesh_material_asset->GetMaterial()->Set("u_MaterialUniforms.AmbientStrength", ambient_strength);

					float& diffuse_strength = mesh_material_asset->GetMaterial()->GetFloat("u_MaterialUniforms.DiffuseStrength");
					if (UI::Property("Diffuse Strength", diffuse_strength, 0.01f, 0.0f, 1.0f))
						mesh_material_asset->GetMaterial()->Set("u_MaterialUniforms.DiffuseStrength", diffuse_strength);

					float& specular_strength = mesh_material_asset->GetMaterial()->GetFloat("u_MaterialUniforms.SpecularStrength");
					if (UI::Property("Specular Strength", specular_strength, 0.01f, 0.0f, 1.0f))
						mesh_material_asset->GetMaterial()->Set("u_MaterialUniforms.SpecularStrength", specular_strength);
				}


				ImGui::PopID();

				UI::EndProperties();
			}

			UI::EndTreeNode();
		}
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

			if (!m_selection_context.HasComponent<CircleRendererComponent>() && ImGui::MenuItem("Circle Renderer"))
			{
				m_selection_context.AddComponent<CircleRendererComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (!m_selection_context.HasComponent<NativeScriptComponent>() && ImGui::MenuItem("Native Script"))
			{
				m_selection_context.AddComponent<NativeScriptComponent>();
				ImGui::CloseCurrentPopup();
			}

			if (!m_selection_context.HasComponent<CSharpScriptComponent>() && ImGui::MenuItem("C# Script"))
			{
				m_selection_context.AddComponent<CSharpScriptComponent>();
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

			if (!m_selection_context.HasComponent<UIPanelComponent>() && ImGui::MenuItem("UI Panel"))
			{
				m_selection_context.AddComponent<UIPanelComponent>();
				ImGui::CloseCurrentPopup();
			}

            // text 2d component
            if (!m_selection_context.HasComponent<TextComponent>() && ImGui::MenuItem("Text"))
            {
                m_selection_context.AddComponent<TextComponent>();
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

		DrawComponent<CameraComponent>("Camera", entity, [](CameraComponent& component)
			{
				SceneCamera& camera = component.Camera;
				UI::BeginProperties();

				UI::Property("Primary", &component.Primary);

				const char* projection_type_strings[] = { "Perspective", "Orthographic" };

				SceneCamera::ProjectionType proj_type = camera.GetProjectionType();
				if (UI::PropertyDropdown("Projection Type", projection_type_strings, 2, proj_type))
					camera.SetProjectionType(proj_type);
				
				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					bool changed = false;
					float fov = glm::degrees(camera.GetPerspectiveVerticalFOV());
					if (UI::Property("FOV", fov, 1.0f, 1.0f, 200.0f))
						camera.SetPerspectiveVerticalFOV(glm::radians(fov));

					float near_clip = camera.GetPerspectiveNearClip();
					if (UI::Property("Near Clip", near_clip, 0.1f, 0.001f, 10000.0f))
						camera.SetPerspectiveNearClip(near_clip);

					float far_clip = camera.GetPerspectiveFarClip();
					if (UI::Property("Far Clip", far_clip, 0.1f, 0.001f, 10000.0f))
						camera.SetPerspectiveFarClip(far_clip);
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					bool changed = false;

					float ortho_size = camera.GetOrthographicSize();
					if (UI::Property("Size", ortho_size, 1.0f, 1.0f, 200.0f))
						camera.SetOrthographicSize(ortho_size);
					
					float near_clip = camera.GetOrthographicNearClip();
					if (UI::Property("Near Clip", near_clip, 0.1f, 0.001f, 10000.0f))
						camera.SetOrthographicNearClip(near_clip);

					float far_clip = camera.GetOrthographicFarClip();
					if (UI::Property("Far Clip", far_clip, 0.1f, 0.001f, 10000.0f))
						camera.SetOrthographicFarClip(far_clip);
				}
				UI::Property("Fixed Aspect Ratio", &component.Fixed_aspect_ratio);

				UI::EndProperties();
			});

		DrawComponent<UIPanelComponent>("UI Panel", entity, [this](UIPanelComponent& component)
			{
				ui::IPanel* panel = component.panel;

				UI::BeginProperties();
				
				if (!panel)
				{
					size_t count = util::calculate_unique_enum_count(ui::panel_type_iterator{});
					std::vector<const char*> enum_names;
					enum_names.reserve(count);

					for (ui::panel_type_t panel_type : ui::panel_type_iterator{})
						enum_names.push_back(panel_type_to_c_str(panel_type));

					UI::PropertyDropdown("Panel Type", enum_names.data(), count, component.panel_type);

					if (ImGui::Button("Create"))
						component.panel = ui::PanelFactory::create_panel(component.panel_type, {});

				}
				else
				{

					glm::vec2& pos = panel->get_position();
					glm::vec2& size = panel->get_size();
					UI::PropertyReadOnlyChars("Panel Type", ui::panel_type_to_c_str(component.panel_type));
					UI::Property("Position", pos);
					UI::Property("Size", size);
					UI::PropertyColorEdit4("Background Color", panel->get_panel_style().background_color);
					//UI::Propert("Render background", panel->get_panel_style().background_color);

					// panel specific ui elements
					switch (panel->get_panel_type())
					{

						case ui::panel_type_t::ImageButton:
							if (UI::PropertyImageButton("Texture", panel->get_panel_style().image, { 32, 32 }, { 0.0f, 1.0f }, { 1.0f, 0.0f }))
							{
								auto filepath = FileDialog::OpenFile("Image File (*.png)\0*.png\0");
								if (!filepath.empty())
								{
									// #TODO go through asset manager
									panel->get_panel_style().image = Texture2D::Create(filepath);
								}
							}
							break;
					}

				}
				
				UI::EndProperties();
			});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [this](SpriteRendererComponent& component)
			{
				UI::BeginProperties();

				UI::PropertyColorEdit4("Tint Color", component.Color);

                const ref<Texture>& white_texture = Application::Get().get_renderer_2d()->get_white_texture();
                ref<Texture> texture_asset = component.Texture != asset::null_asset_id ?
                    asset::get_asset<Texture2D>(component.Texture) : white_texture;
                if (!texture_asset)
                {
                    KB_CORE_ERROR(
                        "[SceneHeirarchyPanel]: Failed to load texture with asset id '{}' for image button. Defaulting to white texture",
                        component.Texture
                    );
                    
                    texture_asset = white_texture;
                }

				if (UI::PropertyImageButton("Texture", texture_asset, {32, 32}, {0.0f, 1.0f}, {1.0f, 0.0f}))
				{
					auto filepath = FileDialog::OpenFile("Image File (*.png)\0*.png\0");
					if (!filepath.empty())
					{
                        const auto& texture_asset = asset::get_asset<Texture2D>(filepath);
						component.Texture = texture_asset->get_id();
					}
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const auto payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const auto path_wchar_str = (const wchar_t*)payload->Data;
						auto path = std::filesystem::path{ ProjectManager::get().get_active()->get_asset_directory_path() / path_wchar_str };
						auto path_str = path.string();
						if (path.extension() == ".png")
                        {
                            const auto& texture_asset = asset::get_asset<Texture2D>(path);
                            if (texture_asset)
                                component.Texture = texture_asset->get_id();
                            else
                            {
                                KB_CORE_ERROR(
                                    "[SceneHeirarchyPanel]: Failed to load texture2d asset from filepath '{}'. Defaulting to null texture id", 
                                    path_str
                                );
                                component.Texture = asset::null_asset_id;
                            }
                        }
						else
							KB_CORE_ERROR("Tried to load non image file as a texture, {0}", path_str);
					}
					ImGui::EndDragDropTarget();
				}

				UI::Property("Visible", &component.Visible);

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

		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [this](CircleRendererComponent& component)
			{
				UI::BeginProperties();

				UI::PropertyColorEdit4("Tint Color", component.Color);
				UI::Property("Radius", component.Radius, 0.1f, 0.0f, 10.0f);
				UI::Property("Thickness", component.Thickness, 0.0025f, 0.0f, 1.0f);
				UI::Property("Fade", component.Fade, 0.00025f, 0.0f, 1.0f);

				UI::EndProperties();
			});

		DrawComponent<NativeScriptComponent>("Native Script", entity, [&](auto& component)
			{
				UI::BeginProperties();
				if (component.Instance)
				{
					std::string name = "placeholder";
					std::vector<std::string> name_vec = Parser::CPP::FindClassAndStructNames( (ProjectManager::get().get_active()->get_project_directory() / std::filesystem::path{ component.GetFilepath() }).string());
					if (!name_vec.empty())
						name = name_vec[0];

					UI::PropertyReadOnlyString("Script Name", name);

					// #TODO relative path based on project
					UI::PropertyReadOnlyString("Filepath", component.GetFilepath());
				}
				else
				{
					if (UI::Button("Add"))
					{
						auto filepath = FileDialog::OpenFile("Header File (*.h)\0*.h\0Source File (*.cpp)\0*.cpp\0");
						KB_CORE_ASSERT(ProjectManager::get().get_active(), "no active project!");
						if (!filepath.empty())
						{
							std::filesystem::path relative_path = std::filesystem::relative(filepath, ProjectManager::get().get_active()->get_project_directory());
							component.BindEditor(filepath);
						}
					}
				}
				UI::EndProperties();
			});

		DrawComponent<CSharpScriptComponent>("C# Script", entity, [&](CSharpScriptComponent& component)
			{
				if (!ProjectManager::get().get_active())
				{
					UI::BeginProperties();
					bool active = false;
					UI::PropertyReadOnlyString("No active project :(", "");
					UI::EndProperties();
					return;
				}

				UI::BeginProperties();
				
				bool is_error = !CSharpScriptEngine::ModuleExists(component.Module_name);
				std::string name = CSharpScriptEngine::StripNamespace(ProjectManager::get().get_active()->GetConfig().CSharp_script_default_namespace, component.Module_name);

				bool was_error = is_error;
				if (was_error)
					ImGui::PushStyleColor(ImGuiCol_Text, { 0.9f, 0.2f, 0.2f, 1.0f });

				if (UI::Property("Module Name", name))
				{
					if (is_error)
						CSharpScriptEngine::ShutdownScriptEntity(entity, component.Module_name);

					if (CSharpScriptEngine::ModuleExists(name))
					{
						component.Module_name = name;
						is_error = false;
					}
					else if (CSharpScriptEngine::ModuleExists(ProjectManager::get().get_active()->GetConfig().CSharp_script_default_namespace + "." + name))
					{
						component.Module_name = ProjectManager::get().get_active()->GetConfig().CSharp_script_default_namespace + "." + name;
						is_error = false;
					}
					else
					{
						component.Module_name = name;
						is_error = true;
					}

					if (!is_error)
						CSharpScriptEngine::InitScriptEntity(entity);
				}

				if (was_error)
					ImGui::PopStyleColor();

				// Exposing public fields
				if (!is_error)
				{
					EntityInstanceData& entity_instance_data = CSharpScriptEngine::GetEntityInstanceData(entity.GetSceneUUID(), entity.GetUUID());
					
				}

				if (UI::Button("Run Script"))
					CSharpScriptEngine::OnCreateEntity(entity);

				UI::EndProperties();
			});

		DrawComponent<MeshComponent>("Mesh", entity, [&](MeshComponent& component)
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

				UI::BeginProperties();

				UI::PushItemDisabled();
				UI::Property("Filename:", component.Filepath.c_str());
				UI::PopItemDisabled();

				IntrusiveRef<Mesh> mesh = component.Mesh;
				if (mesh)
				{
					for (uint32_t submesh_index : mesh->GetSubmeshes())
					{
						UI::Property("Submesh Index", submesh_index);
					}

					DrawMaterialTable<MeshComponent>(mesh->GetMaterials());
				}

				UI::EndProperties();
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
				int32_t index = static_cast<int32_t>(std::find(options.begin(), options.end(), KablunkRigidBodyTypeToString(component.Type)) - options.begin());
				if (UI::PropertyDropdown("Type", options, options.size(), &index))
					component.Type = StringToKablunkRigidBodyType(options[index]);
				
				UI::Property("Fixed Rotation", &component.Fixed_rotation);
				UI::Property("Affected by Gravity", &component.Does_gravity_affect);

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

        DrawComponent<TextComponent>("Text", entity, [&](TextComponent& component)
            {
                UI::BeginProperties();

                UI::Property("Text String", component.m_text_str);
                UI::PropertyColorEdit4("Color", component.m_tint_color);

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
