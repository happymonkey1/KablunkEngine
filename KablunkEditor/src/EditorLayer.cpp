#include "EditorLayer.h"

#include "Kablunk/Core/Application.h"
#include "Kablunk/Core/MouseCodes.h"
#include "Kablunk/Core/Uuid64.h"
#include "Kablunk/Core/JobSystem.h"

#include "Kablunk/Utilities/PlatformUtils.h"

#include "Kablunk/Math/Math.h"

#include "Kablunk/Imgui/ImGuiWrappers.h"

#include <Kablunk/Scene/SceneSerializer.h>

#include "Kablunk/Scripts/CSharpScriptEngine.h"

#include "Kablunk/Utilities/Parser.h"

#include "Kablunk/Project/Project.h"
#include "Kablunk/Project/ProjectSerializer.h"

// #TODO replace when runtime is figured out
#include "Sandbox/Core.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <ImGuizmo.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>



#define DISABLE_NATIVE_SCRIPTING 0

namespace Kablunk
{

	constexpr uint32_t MAX_PROJECT_NAME_LENGTH = 255;
	constexpr uint32_t MAX_PROJECT_FILEPATH_LENGTH = 512;

	static char* s_project_name_buffer = new char[MAX_PROJECT_NAME_LENGTH];
	static char* s_project_filepath_buffer = new char[MAX_PROJECT_FILEPATH_LENGTH];
	static std::string s_kablunk_install_path = "";


	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_editor_camera{ 45.0f, 1.778f, 0.1f, 1000.0f }, m_project_properties_panel{ nullptr }
	{
		

		m_icon_play = Texture2D::Create("Resources/icons/play_icon.png");
		m_icon_stop = Texture2D::Create("Resources/icons/stop_icon.png");

		memset(s_project_filepath_buffer, 0, MAX_PROJECT_FILEPATH_LENGTH);
		memset(s_project_name_buffer, 0, MAX_PROJECT_NAME_LENGTH);
	}

	void EditorLayer::OnAttach()
	{
		//m_missing_texture		= Texture2D::Create("assets/textures/missing_texture.png");
		//m_kablunk_logo		= AssetManager::Create<Texture2D>("assets/textures/kablunk_logo.png");
		//m_icon_play			= Texture2D::Create("assets/icons/round_play_arrow_white_72dp.png");

		FramebufferSpecification frame_buffer_spec;
		auto window_dimensions = Application::Get().GetWindowDimensions();
		frame_buffer_spec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		frame_buffer_spec.Width  = window_dimensions.x;
		frame_buffer_spec.Height = window_dimensions.y;
		m_frame_buffer = Framebuffer::Create(frame_buffer_spec);


		m_editor_scene = CreateRef<Scene>();
		m_active_scene = m_editor_scene;

		m_scene_hierarchy_panel.SetContext(m_active_scene);


		s_kablunk_install_path = FileSystem::GetEnvironmentVar("KABLUNK_DIR");
		KB_CORE_INFO("Kablunk install path: '{0}'", s_kablunk_install_path);
	}

	void EditorLayer::OnDetach()
	{

	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		KB_PROFILE_FUNCTION();

		// ==========
		//   Update
		// ==========
		
		//if (m_viewport_focused) m_editor_camera.OnUpdate(ts);

		//m_content_browser_panel.OnUpdate(ts);

		if (m_imgui_profiler_stats.Counter >= m_imgui_profiler_stats.Counter_max)
		{
			float miliseconds = ts.GetMiliseconds();
			m_imgui_profiler_stats.Delta_time = miliseconds;
			m_imgui_profiler_stats.Fps = 1000.0f / miliseconds;
			m_imgui_profiler_stats.Counter -= m_imgui_profiler_stats.Counter_max;
		}
		else
			m_imgui_profiler_stats.Counter += ts.GetMiliseconds() / 1000.0f;

		// #TODO move to a scene renderer
		auto spec = m_frame_buffer->GetSpecification();
		if (m_viewport_size.x > 0.0f && m_viewport_size.y > 0.0f 
			&& (spec.Width != m_viewport_size.x || spec.Height != m_viewport_size.y))
		{
			m_frame_buffer->Resize(static_cast<uint32_t>(m_viewport_size.x), static_cast<uint32_t>(m_viewport_size.y));

			m_editor_camera.OnViewportResize(m_viewport_size.x, m_viewport_size.y);
			m_active_scene->OnViewportResize(static_cast<uint32_t>(m_viewport_size.x), static_cast<uint32_t>(m_viewport_size.y));
		}
		
		// ==========
		//   Render
		// ==========


		

		Renderer2D::ResetStats();

		// #TODO move to a scene renderer
		m_frame_buffer->Bind();

		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		// Clear our entity ID buffer to -1
		m_frame_buffer->ClearAttachment(1, -1);

		switch (m_scene_state)
		{
		case SceneState::Edit:

			m_editor_camera.OnUpdate(ts);

			m_active_scene->OnUpdateEditor(ts, m_editor_camera);

			ViewportClickSelectEntity();
			break;
		case SceneState::Play:

			m_active_scene->OnUpdateRuntime(ts);

			break;
		}

		OnOverlayRender();

		m_frame_buffer->Unbind();

#if KB_NATIVE_SCRIPTING
		NativeScriptEngine::Get()->OnUpdate(ts);
#endif
		
	}

	void EditorLayer::OnImGuiRender(Timestep ts)
	{
		KB_PROFILE_FUNCTION();

		static bool opt_fullscreen	= true;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}


		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 1.0f, 1.0f });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, { 0.0f, 0.0f, 0.0f, 0.0f });
		ImGui::Begin("##kablunk_editor_dockspace", nullptr, window_flags);
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);


		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		auto& style = ImGui::GetStyle();
		float min_window_size = style.WindowMinSize.x;
		style.WindowMinSize.x = 375.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("kablunk_editor_dockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = min_window_size;

		UI_DrawMenuBar();
		
		//ImGui::PopStyleVar();

		//ImGui::Dummy({ 0.0f, 200.0f });

		m_scene_hierarchy_panel.OnImGuiRender();
		m_content_browser_panel.OnImGuiRender();

		if (Project::GetActive().get() != nullptr)
			m_project_properties_panel.OnImGuiRender(m_show_project_properties_panel);
		else
			m_show_project_properties_panel = false;

		if (m_show_debug_panel)
		{
			ImGui::Begin("Debug Panel");

			UI::BeginProperties();

			UI::PropertyReadOnlyFloat("Frame time", m_imgui_profiler_stats.Delta_time);
			UI::PropertyReadOnlyFloat("FPS", m_imgui_profiler_stats.Fps);

			Renderer2D::Renderer2DStats stats = Kablunk::Renderer2D::GetStats();

			UI::PropertyReadOnlyUint32("Draw Calls", stats.Draw_calls);
			UI::PropertyReadOnlyUint32("Verts", stats.GetTotalVertexCount());
			UI::PropertyReadOnlyUint32("Indices", stats.GetTotalIndexCount());
			UI::PropertyReadOnlyUint32("Quad Count", stats.Quad_count);

			UI::Property("Editor Selected Entity", m_selected_entity.GetHandleAsString());
			UI::Property("Hierarchy Panel Selected Entity", m_scene_hierarchy_panel.GetSelectedEntity().GetHandleAsString());
			UI::Property("Show Physics Colliders", &m_show_physics_colliders);

			UI::EndProperties();

			ImGui::End();
		}
		

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
		if (ImGui::Begin("Viewport"))
		{
			auto viewport_min_region = ImGui::GetWindowContentRegionMin();
			auto viewport_max_region = ImGui::GetWindowContentRegionMax();
			auto viewport_offset = ImGui::GetWindowPos();
			m_viewport_bounds[0] = { viewport_min_region.x + viewport_offset.x, viewport_min_region.y + viewport_offset.y };
			m_viewport_bounds[1] = { viewport_max_region.x + viewport_offset.x, viewport_max_region.y + viewport_offset.y };

			m_viewport_focused = ImGui::IsWindowFocused();
			m_viewport_hovered = ImGui::IsWindowHovered();
			Application::Get().GetImGuiLayer()->SetAllowEventPassing(m_viewport_focused || m_viewport_hovered);

			auto panel_size = ImGui::GetContentRegionAvail();
			auto width = panel_size.x, height = panel_size.y;
			m_viewport_size = { width, height };

			auto frame_buffer_id = m_frame_buffer->GetColorAttachmentRendererID();
			ImGui::Image(
				reinterpret_cast<void*>(static_cast<uint64_t>(frame_buffer_id)),
				{ m_viewport_size.x, m_viewport_size.y },
				{ 0.0f, 1.0f }, { 1.0f, 0.0f }
			);

			if (ImGui::BeginDragDropTarget())
			{
				if (const auto payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const auto path_wchar_str = (const wchar_t*)payload->Data;
					auto path = std::filesystem::path{ Project::GetAssetDirectory() / path_wchar_str };
					if (path.extension() == FILE_EXTENSIONS::KABLUNK_SCENE)
						OpenScene(path);
					else if (path.extension() == FILE_EXTENSIONS::FBX)
					{
						auto entity = m_active_scene->CreateEntity("Untitled Model");
						auto& mesh_comp = entity.AddComponent<MeshComponent>();
						mesh_comp.LoadMeshFromFileEditor(path.string(), entity);
					}
					else
						KB_CORE_ERROR("Tried to load non kablunkscene file as scene");
				}
				ImGui::EndDragDropTarget();
			}

			// ImGuizmo

			// #TODO refactor to use callbacks instead of querying current scene
			auto selected_entity = m_scene_hierarchy_panel.GetSelectedEntity();
			if (selected_entity && m_gizmo_type != -1)
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();

				float window_width = m_viewport_bounds[1].x - m_viewport_bounds[0].x;
				float window_height = m_viewport_bounds[1].y - m_viewport_bounds[0].y;
				ImGuizmo::SetRect(m_viewport_bounds[0].x, m_viewport_bounds[0].y, window_width, window_height);

				// Runtime camera
				/*
				auto camera_entity = m_active_scene->GetPrimaryCameraEntity();
				const auto& camera = camera_entity.GetComponent<CameraComponent>().Camera;
				const auto& camera_projection = camera.GetProjection();
				auto camera_view = glm::inverse(camera_entity.GetComponent<TransformComponent>().GetTransform());
				*/

				// Editor Camera
				const auto& camera_projection = m_editor_camera.GetProjection();
				auto camera_view = m_editor_camera.GetViewMatrix();

				// Selected Entity Transform
				auto& transform_component = selected_entity.GetComponent<TransformComponent>();
				auto transform = transform_component.GetTransform();

				// Snapping
				bool snap = Input::IsKeyPressed(Key::LeftControl);
				float snap_value = (m_gizmo_type == ImGuizmo::ROTATE) ? 45.0f : 0.5f;

				float snap_values[3] = { snap_value, snap_value, snap_value };

				ImGuizmo::Manipulate(glm::value_ptr(camera_view),
					glm::value_ptr(camera_projection),
					static_cast<ImGuizmo::OPERATION>(m_gizmo_type),
					ImGuizmo::LOCAL,
					glm::value_ptr(transform),
					nullptr,
					snap ? snap_values : nullptr
				);


				if (ImGuizmo::IsUsing())
				{
					auto original_rotation = transform_component.Rotation;
					glm::vec3 translation, scale, rotation;
					if (Math::decompose_transform(transform, translation, scale, rotation))
					{
						// Translation
						transform_component.Translation = translation;

						// Scale
						transform_component.Scale = scale;

						// Rotation
						auto delta_rotation = rotation - transform_component.Rotation;
						transform_component.Rotation += delta_rotation;
					}
				}
			}
			else if (selected_entity && m_gizmo_type == -1)
			{
				if (selected_entity.HasComponent<PointLightComponent>())
				{
					// #TODO add guizmo for point light
				}
			}


			ImGui::PopStyleVar();
			ImGui::End();
		}
		
		CSharpScriptEngine::OnImGuiRender();

		UI_Toolbar();
		UI_KablunkInstallPopup();

		if (m_show_create_new_project_popup)
		{
			ImGui::OpenPopup("New Project");
			memset(s_project_filepath_buffer, 0, MAX_PROJECT_FILEPATH_LENGTH);
			memset(s_project_name_buffer, 0, MAX_PROJECT_NAME_LENGTH);
			m_show_create_new_project_popup = false;
		}

		auto bold_font = io.Fonts->Fonts[0];
		auto center = ImGui::GetMainViewport()->GetCenter();

		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, { 0.5f, 0.5f });
		ImGui::SetNextWindowSize({ 700, 350 });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 20, 20 });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 4, 10 });
		if (ImGui::BeginPopupModal("New Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
		{
			UI::ShiftCursorY(350.0f / 8.0f);
			UI::BeginProperties();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 10, 7 });
			ImGui::PushFont(bold_font);
			std::string full_project_path = strlen(s_project_filepath_buffer) > 0
				? std::string(s_project_filepath_buffer) + "/" + std::string(s_project_name_buffer) : "";
			UI::PropertyReadOnlyString("Full Project Path", full_project_path);
			ImGui::PopFont();

			UI::Property("Project Name", s_project_name_buffer, MAX_PROJECT_NAME_LENGTH);
			
			auto label_size = ImGui::CalcTextSize("...", nullptr, true);
			auto& style = ImGui::GetStyle();
			auto button_size = ImGui::CalcItemSize(
				{ 0, 0 }, label_size.x + style.FramePadding.x * 2.0f, 
				label_size.y + style.FramePadding.y * 2.0f
			);
			
			UI::PropertyReadOnlyChars("Project Location", s_project_filepath_buffer);
		
			auto px = ImGui::GetColumnWidth() - button_size.x / 2.0f - style.FramePadding.x * 2.0f - style.ItemInnerSpacing.x - 1;
			ImGui::NextColumn();
			UI::ShiftCursorX(px);
			if (ImGui::Button("..."))
			{
				std::string path = FileDialog::OpenFolder();
				std::replace(path.begin(), path.end(), '\\', '/');
				memcpy(s_project_filepath_buffer, path.data(), path.length());
			}

			ImGui::Separator();
			UI::EndProperties();

			auto create_text_size = ImGui::CalcTextSize("Create", nullptr, true);
			auto create_button_size = ImGui::CalcItemSize(
				{ 0, 0 }, create_text_size.x + style.FramePadding.x * 2.0f,
				label_size.y + style.FramePadding.y * 2.0f
			);

			auto cancel_text_size = ImGui::CalcTextSize("Cancel", nullptr, true);
			auto cancel_button_size = ImGui::CalcItemSize(
				{ 0, 0 }, cancel_text_size.x + style.FramePadding.x * 2.0f,
				label_size.y + style.FramePadding.y * 2.0f
			);

			auto button_widths = create_button_size.x / 2.0f + (style.ItemSpacing.x - 1) + cancel_button_size.x / 2.0f;
			UI::ShiftCursorX((600.0f / 2.0f) - button_widths / 2.0f - style.FramePadding.x);
			ImGui::PushFont(bold_font);
			if (ImGui::Button("Create"))
			{
				CreateProject(full_project_path);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopFont();
			ImGui::PopStyleVar();


			ImGui::EndPopup();
		}
		else
		{
			if (strlen(s_project_filepath_buffer) > 0)
				OpenProject(s_project_filepath_buffer);
		}


		ImGui::PopStyleVar(2);

		ImGui::End();
	}

	void EditorLayer::UI_DrawMenuBar()
	{
		float main_menu_window_height{ -1.0f };
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Create Project..."))
					m_show_create_new_project_popup = true;
				if (ImGui::MenuItem("Save Project"))
					SaveProject();
				if (ImGui::MenuItem("Open Project...", "Crtl+O"))
					OpenProject();

				ImGui::Separator();
				if (ImGui::MenuItem("New Scene", "Crtl+N"))
					NewScene();

				if (ImGui::MenuItem("Open Scene..."))
					OpenScene();

				if (ImGui::MenuItem("Save Scene", "Crtl+S"))
					SaveScene();
			

				if (ImGui::MenuItem("Save Scene As...", "Crtl+Shift+S"))
					SaveSceneAs();

				if (ImGui::MenuItem("Exit"))
					Application::Get().Close();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{

				if (!Project::GetActive())
				{
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				}

				if (ImGui::MenuItem("Project Settings"))
					m_show_project_properties_panel = true;

#if KB_NATIVE_SCRIPTING
				if (ImGui::MenuItem("Update Project Engine Files"))
					UpdateProjectEngineFiles();

				if (ImGui::MenuItem("Reload NativeScript DLLs"))
					NativeScriptEngine::Get()->LoadDLLRuntime(Project::GetNativeScriptModuleFileName(), Project::GetNativeScriptModulePath().string());
#endif

				if (ImGui::MenuItem("Reload C# Assemblies"))
					CSharpScriptEngine::ReloadAssembly(Project::GetCSharpScriptModuleFilePath());
				
				if (!Project::GetActive())
				{
					ImGui::PopStyleVar();
					ImGui::PopItemFlag();
				}

				if (ImGui::MenuItem("Select KablunkEngine Install"))
					m_show_replace_kablunk_install_popup = true;

				ImGui::EndMenu();
			}

			main_menu_window_height = ImGui::GetFrameHeight();
			ImGui::EndMenuBar();
		}

	}

	void EditorLayer::UI_Toolbar()
	{
		auto flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 1 });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 0, 0 });
		ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.3f, 0.305f, 0.31f, 0.5f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.15f, 0.1505f, 0.151f, 0.5f });
		ImGui::Begin("##toolbar", nullptr, flags);

		auto icon = m_scene_state == SceneState::Edit ? m_icon_play : m_icon_stop;
		float size = ImGui::GetWindowHeight() - 4.0f;

		ImGui::SameLine((ImGui::GetWindowContentRegionMax().x / 2.0f) - (1.5f * (ImGui::GetFontSize() + ImGui::GetStyle().ItemSpacing.x)) - (size / 2.0f));
		if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { size, size }, { 0, 0 }, { 1, 1 }, 0))
		{
			if (m_scene_state == SceneState::Edit)
				OnScenePlay();
			else if (m_scene_state == SceneState::Play)
				OnSceneStop();
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);

		ImGui::End();
	}

	void EditorLayer::UI_KablunkInstallPopup()
	{
		if ((m_show_replace_kablunk_install_popup || s_kablunk_install_path.empty()) 
			&& !ImGui::IsPopupOpen("Select KablunkEngine Install"))
		{
			ImGui::OpenPopup("Select KablunkEngine Install");
			s_kablunk_install_path.reserve(MAX_PROJECT_FILEPATH_LENGTH);
		}

		auto center = ImGui::GetMainViewport()->GetCenter();

		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, { 0.5f, 0.5f });
		ImGui::SetNextWindowSize({ 700, 350 });
		if (ImGui::BeginPopupModal("Select KablunkEngine Install", nullptr, ImGuiWindowFlags_NoMove | ImGuiTableColumnFlags_NoResize))
		{
			if (s_kablunk_install_path.empty())
			{
				const auto& io = ImGui::GetIO();
				auto bold_font = io.Fonts->Fonts[0];
				ImGui::PushFont(bold_font);
				ImGui::TextUnformatted("Could not find KablunkEngine Install");
				ImGui::PopFont();
			}

			ImGui::TextWrapped("Please select the root folder for the KablunkEngine install you want to use");

			ImGui::Dummy({ 0, 8 });

			auto label_size = ImGui::CalcTextSize("...", nullptr, true);
			auto& style = ImGui::GetStyle();
			auto button_size = ImGui::CalcItemSize({ 0, 0 }, label_size.x + style.FramePadding.x * 2.0f - style.ItemInnerSpacing.x - 1, label_size.y + style.FramePadding.y * 2.0f);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 2, 10 });
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 6 });
			ImGui::SetNextItemWidth(700 - button_size.x - style.FramePadding.x * 2.0f - style.ItemInnerSpacing.x - 1);
			
			ImGui::InputTextWithHint("##kablunk_install_location_text", 
				"Select Install", s_kablunk_install_path.data(), MAX_PROJECT_FILEPATH_LENGTH, ImGuiInputTextFlags_ReadOnly);

			ImGui::SameLine();
			if (ImGui::Button("..."))
			{
				std::string path = FileDialog::OpenFolder();
				s_kablunk_install_path.assign(path);
			}

			if (ImGui::Button("Confirm"))
			{
				bool success = FileSystem::SetEnvironmentVar("KABLUNK_DIR", s_kablunk_install_path.c_str());
				KB_CORE_ASSERT(success, "Failed to set environment variable!");
				m_show_replace_kablunk_install_popup = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				m_show_replace_kablunk_install_popup = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::PopStyleVar(2);

			ImGui::EndPopup();
		}
	}

	void EditorLayer::OnScenePlay()
	{
		m_scene_state = SceneState::Play;
		//m_active_scene->OnStartRuntime();
		
		if (Project::GetActive())
			if (Project::GetActive()->GetConfig().Reload_csharp_script_assemblies_on_play)
				CSharpScriptEngine::ReloadAssembly(Project::GetCSharpScriptModuleFilePath());

		m_runtime_scene = Scene::Copy(m_active_scene);
		m_runtime_scene->OnStartRuntime();

		m_active_scene = m_runtime_scene;
		m_selected_entity = {};
	}

	void EditorLayer::OnSceneStop()
	{
		m_scene_state = SceneState::Edit;

		m_runtime_scene->OnStopRuntime();
		m_runtime_scene.reset();

		CSharpScriptEngine::SetSceneContext(m_editor_scene.get());

		m_active_scene = m_editor_scene;
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_editor_camera.OnEvent(e);

		if (e.GetEventType() == EventType::WindowMinimized)
			m_viewport_size = { 0.0f, 0.0f };

		EventDispatcher dispatcher{ e };
		dispatcher.Dispatch<KeyPressedEvent>(KABLUNK_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(KABLUNK_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// Shortcuts
		if (e.GetRepeatCount() > 0)
			return false;

		bool ctrl_pressed  = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shift_pressed = Input::IsKeyPressed(Key::LeftShift)   || Input::IsKeyPressed(Key::RightShift);

		switch (e.GetKeyCode())
		{
		case Key::N:
		{
			if (ctrl_pressed)
				NewScene();

			break;
		}
		case Key::O:
		{
			if (ctrl_pressed)
				OpenProject();

			break;
		}
		case Key::S:
		{
			if (ctrl_pressed)
			{
				if (shift_pressed)
					SaveSceneAs();
				else
					SaveScene();
			}

			break;
		}
		case Key::D:
		{
			if (ctrl_pressed)
				if (Entity selection = m_scene_hierarchy_panel.GetSelectedEntity(); selection)
					m_active_scene->DuplicateEntity(selection);

			break;
		}

		// Gizmos
		case Key::Q:
		{
			if (!ImGuizmo::IsUsing()) 
				m_gizmo_type = -1;
			break;
		}
		case Key::W:
		{
			if (!ImGuizmo::IsUsing()) 
				m_gizmo_type = ImGuizmo::OPERATION::TRANSLATE;
			break;
		}
		case Key::E:
		{
			if (!ImGuizmo::IsUsing()) 
				m_gizmo_type = ImGuizmo::OPERATION::ROTATE;
			break;
		}
		case Key::R:
		{
			if (!ImGuizmo::IsUsing()) 
				m_gizmo_type = ImGuizmo::OPERATION::SCALE;
			break;
		}


		default:
			break;
		}

		return true;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == Mouse::ButtonLeft)
			if (CanPickFromViewport() && m_scene_state != SceneState::Play)
			{
				m_scene_hierarchy_panel.SetSelectionContext(m_selected_entity);
				// #TODO ray cast mouse picking?

			}

		return false;
	}

	void EditorLayer::NewScene()
	{
		m_editor_scene = CreateRef<Scene>();
		m_editor_scene->OnViewportResize(static_cast<uint32_t>(m_viewport_size.x), static_cast<uint32_t>(m_viewport_size.y));
		
		m_scene_hierarchy_panel.SetContext(m_editor_scene);
		CSharpScriptEngine::SetSceneContext(m_editor_scene.get());

		m_active_scene = m_editor_scene;

		m_editor_scene_path = std::filesystem::path{};
	}

	void EditorLayer::SaveScene()
	{
		if (!m_editor_scene_path.empty())
			SerializeScene(m_editor_scene, m_editor_scene_path);
		else
			SaveSceneAs();
	}

	void EditorLayer::SaveSceneAs()
	{
		auto filepath = FileDialog::SaveFile("Kablunk Scene (*.kablunkscene)\0*.kablunkscene\0");
		if (!filepath.empty())
		{
			SerializeScene(m_editor_scene, filepath);
			m_editor_scene_path = filepath;

			// update default scene for project
			if (Project::GetActive())
			{
				if (Project::GetStartSceneName() == "$SCENE_NAME$")
				{
					Project::SetStartSceneName(m_editor_scene_path.filename().stem().string());
					ProjectSerializer project_serializer{ Project::GetActive() };
					project_serializer.Serialize(Project::GetActive()->GetConfig().Project_directory + "/" + Project::GetActive()->GetConfig().Project_filename);
				}
			}
		}
	}

	void EditorLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& path)
	{
		SceneSerializer serializer{ scene };
		serializer.Serialize(path.string());
	}

	void EditorLayer::OpenScene()
	{
		if (m_scene_state != SceneState::Edit)
			OnSceneStop();

		auto filepath = FileDialog::OpenFile("Kablunk Scene (*.kablunkscene)\0*.kablunkscene\0");
		if (!filepath.empty())
			OpenScene(filepath);
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		NewScene();

		Ref<Scene> new_scene = CreateRef<Scene>();
		auto serializer = SceneSerializer{ new_scene };
		if (serializer.Deserialize(path.string()))
		{
			m_editor_scene = new_scene;
			m_editor_scene->OnViewportResize(static_cast<uint32_t>(m_viewport_size.x), static_cast<uint32_t>(m_viewport_size.y));
			
			m_scene_hierarchy_panel.SetContext(m_editor_scene);
			CSharpScriptEngine::SetSceneContext(m_editor_scene.get());

			m_active_scene = m_editor_scene;

			m_editor_scene_path = path;
		}
	}

	bool EditorLayer::CanPickFromViewport() const
	{
		return m_viewport_hovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(EditorCamera::Camera_control_key);
	}

	void EditorLayer::CreateProject(std::filesystem::path project_path)
	{
		if (!FileSystem::HasEnvironmentVariable("KABLUNK_DIR"))
		{
			KB_CORE_ERROR("KablunkEngine install location not set!");
			return;
		}

		if (project_path.empty())
		{
			KB_CORE_ERROR("Tried creating project with empty path!");
			return;
		}

		if (!std::filesystem::exists(project_path))
			std::filesystem::create_directories(project_path);

		std::filesystem::path resources_path = std::filesystem::path{ s_kablunk_install_path } / "KablunkEditor" / "resources";
		std::filesystem::copy(resources_path / "new_project_template", project_path, std::filesystem::copy_options::recursive);

		{
			// Kablunk Project
			{
				std::ifstream stream{ project_path / "Project.kablunkproj" };
				std::stringstream ss;
				ss << stream.rdbuf();

				stream.close();

				std::string data = ss.str();
				ReplaceToken("$PROJECT_NAME$", data, s_project_name_buffer);

				std::ofstream ostream{ project_path / "Project.kablunkproj" };
				ostream << data;
				ostream.close();
			}

			// Premake
			{
				std::ifstream stream{ project_path / "premake5.lua" };
				std::stringstream ss;
				ss << stream.rdbuf();

				std::string data = ss.str();
				ReplaceToken("$PROJECT_NAME$", data, s_project_name_buffer);
				ReplaceToken("$KABLUNK_DIR$", data, s_kablunk_install_path);
				std::replace(data.begin(), data.end(), '\\', '/');

				std::ofstream ostream{ project_path / "premake5.lua" };
				ostream << data;
				ostream.close();
			}

#if KB_NATIVE_SCRIPTING
			// Generate NativeScript batch
			{
				std::ifstream stream{ project_path / "Windows-CreateNativeScriptProject.bat" };
				std::stringstream ss;
				ss << stream.rdbuf();

				std::string data = ss.str();
				std::string project_path_windows = "\'" + project_path.string();
				std::replace(project_path_windows.begin(), project_path_windows.end(), '/', '\\');
				project_path_windows += "\'";
				ReplaceToken("$PROJECT_DIR$", data, project_path_windows);

				std::ofstream ostream{ project_path / "Windows-CreateNativeScriptProject.bat" };
				ostream << data;
				ostream.close();
			}
#endif


			std::string new_project_filename = std::string{ s_project_name_buffer } + ".kablunkproj";
			std::filesystem::rename(project_path / "Project.kablunkproj", project_path / new_project_filename);

			std::filesystem::create_directories(project_path / "assets" / "scenes");
			std::filesystem::create_directories(project_path / "assets" / "scripts" / "source");
			std::filesystem::create_directories(project_path / "assets" / "textures");
			std::filesystem::create_directories(project_path / "assets" / "audio"); 
			std::filesystem::create_directories(project_path / "assets" / "materials");
			std::filesystem::create_directories(project_path / "assets" / "meshes");

#if DISABLE_NATIVE_SCRIPTING
			// Native scripts
			std::filesystem::create_directories(project_path / "assets" / "bin");
			std::filesystem::create_directories(project_path / "include");
			std::filesystem::create_directories(project_path / "src");
			std::filesystem::create_directories(project_path / "KablunkEngine" / "engine");
			std::filesystem::create_directories(project_path / "KablunkEngine" / "vendor");
			std::filesystem::create_directories(project_path / "KablunkEngine" / "bin");

			std::filesystem::copy(std::filesystem::path{ s_kablunk_install_path } / "bin", project_path / "KablunkEngine" / "bin", std::filesystem::copy_options::recursive);
			std::filesystem::copy(std::filesystem::path{ s_kablunk_install_path } / "KablunkEngine/include", project_path / "KablunkEngine" / "engine", std::filesystem::copy_options::recursive);

			// #TODO replace with better code
			std::string get_vendor_includes_path = "\"" + s_kablunk_install_path;
			std::replace(get_vendor_includes_path.begin(), get_vendor_includes_path.end(), '/', '\\');
			get_vendor_includes_path += "\\scripts\\CopyVendorIncludes.py\"";
			std::string run_python_cmd = "python " + get_vendor_includes_path + " "
				+ "\"" + std::filesystem::path{project_path / "KablunkEngine" / "vendor"}.string() + "\"";

			Threading::JobSystem::AddJob([&run_python_cmd]()
				{
					system(run_python_cmd.c_str());
				});

			

			std::string gen_proj_batch = "\"" + project_path.string();
			std::replace(gen_proj_batch.begin(), gen_proj_batch.end(), '/', '\\');
			gen_proj_batch += "\\Windows-CreateNativeScriptProject.bat\"";
			
			//KB_CORE_WARN("PLEASE RUN 'Windows-CreateNativeScriptProject.bat' YOURSELF!");
			Threading::JobSystem::AddJob([&gen_proj_batch]()
				{
					system(gen_proj_batch.c_str());
				});
#endif
			std::string gen_proj_batch = "\"" + project_path.string();
			std::replace(gen_proj_batch.begin(), gen_proj_batch.end(), '/', '\\');
			gen_proj_batch += "\\Windows-CreateCSharpScriptProject.bat\"";

			system(gen_proj_batch.c_str());
			//Threading::JobSystem::AddJob([&gen_proj_batch]() { system(gen_proj_batch.c_str()); });

			OpenProject(project_path.string() + "/" + std::string{ s_project_name_buffer } + ".kablunkproj");
		}
	}

	void EditorLayer::UpdateProjectEngineFiles()
	{
		auto project = Project::GetActive();
		if (!project)
			return;
		
		auto copy_option_flags = std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing;
		std::filesystem::copy(std::filesystem::path{ s_kablunk_install_path } / "bin", 
			project->GetProjectDirectory() / "KablunkEngine" / "bin", copy_option_flags);
		std::filesystem::copy(std::filesystem::path{ s_kablunk_install_path } / "KablunkEngine/include", 
			project->GetProjectDirectory() / "KablunkEngine" / "engine", copy_option_flags);
	}

	void EditorLayer::OpenProject(const std::string& filepath)
	{
		if (Project::GetActive())
			CloseProject();

		Ref<Project> project = CreateRef<Project>();
		ProjectSerializer serializer{ project };

		serializer.Deserialize(filepath);
		Project::SetActive(project);

		CSharpScriptEngine::LoadAppAssembly(Project::GetCSharpScriptModuleFilePath());

		m_content_browser_panel.SetCurrentDirectory(Project::GetAssetDirectoryPath());
		m_project_properties_panel = ProjectPropertiesPanel{ project };

		if (const std::string& scene_name = project->GetStartSceneName(); !scene_name.empty())
			OpenScene(Project::GetAssetDirectoryPath() / "scenes" / scene_name);
		else
			NewScene();

		m_scene_hierarchy_panel.ClearSelectionContext();

		memset(s_project_name_buffer, 0, MAX_PROJECT_NAME_LENGTH);
		memset(s_project_filepath_buffer, 0, MAX_PROJECT_FILEPATH_LENGTH);
	}

	void EditorLayer::OpenProject()
	{
		auto filepath = FileDialog::OpenFile("Kablunk Project (*.kablunkproj)\0*.kablunkproj\0");
		
		if (filepath.empty())
			return;

		strcpy_s(s_project_filepath_buffer, MAX_PROJECT_FILEPATH_LENGTH, filepath.data());
	}

	void EditorLayer::SaveProject()
	{
		auto& project = Project::GetActive();
		if (!project)
		{
			KB_CORE_ERROR("SaveProject called without active project!");
			return;
		}

		ProjectSerializer serializer{ project };
		serializer.Serialize(project->GetConfig().Project_directory + "/" + project->GetConfig().Project_filename);
	}

	void EditorLayer::CloseProject(bool unload /*= true*/)
	{
		SaveProject();

		CSharpScriptEngine::SetSceneContext(nullptr);

		m_scene_hierarchy_panel.SetContext(nullptr);
		m_active_scene = nullptr;

		if (unload)
			Project::SetActive(nullptr);
	}

	void EditorLayer::ReplaceToken(const char* token, std::string& data, const std::string& new_token)
	{
		size_t p = 0;
		while ((p = data.find(token, p)) != std::string::npos)
		{
			data.replace(p, strlen(token), new_token);
			p += strlen(token);
		}
	}

	// #TODO Currently streams a second full viewport width and height framebuffer from GPU to use for mousepicking.
	//		 Consider refactoring to only stream a 3x3 framebuffer around the mouse click to save on bandwidth 
	void EditorLayer::ViewportClickSelectEntity()
	{
		// #TODO clicking outside viewport still tries to select entity, causing a deselection in most cases

		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_viewport_bounds[0].x;
		my -= m_viewport_bounds[0].y;

		glm::vec2 viewport_size = m_viewport_bounds[1] - m_viewport_bounds[0];
		my = m_viewport_size.y - my;

		auto mouse_x = static_cast<int>(mx);
		auto mouse_y = static_cast<int>(my);

		//KB_CORE_TRACE("Mouse: {0}, {1}", mouse_x, mouse_y);
		int pixel_data = m_frame_buffer->ReadPixel(1, mouse_x, mouse_y);
		
		//KB_CORE_TRACE("Mouse picked entity id: {0}", pixel_data);
		
		if (mouse_x >= 0 && mouse_y >= 0 && mouse_x < static_cast<int>(viewport_size.x) && mouse_y < static_cast<int>(viewport_size.y))
		{
			if (pixel_data != -1)
			{
				EntityHandle handle{ static_cast<uint64_t>(pixel_data) };
				m_selected_entity = { handle, m_active_scene.get() };
			}
			else // Make sure we are not trying to use a gizmo and we are not using the editor camera
				m_selected_entity = {};
		}
	}

	void EditorLayer::OnOverlayRender()
	{
		if (m_scene_state == SceneState::Play)
		{
			auto cam_entity = m_active_scene->GetPrimaryCameraEntity();
			if (cam_entity.Valid())
				Renderer2D::BeginScene(cam_entity.GetComponent<CameraComponent>().Camera, cam_entity.GetComponent<TransformComponent>().GetTransform());
			else
			{
				KB_CORE_ERROR("Cannot render overlay in runtime scene because there is no main camera!");
				return;
			}
		}
		else
		{
			Renderer2D::BeginScene(m_editor_camera);
		}

		if (m_show_physics_colliders)
		{
			auto view = m_active_scene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
			for (auto e : view)
			{
				auto& [transform_comp, cc2D_comp] = view.get<TransformComponent, CircleCollider2DComponent>(e);
				auto translate = transform_comp.Translation + glm::vec3{ cc2D_comp.Offset, 0.001f };
				auto scale = transform_comp.Scale * glm::vec3{ cc2D_comp.Radius * 2.0f };

				auto transform = glm::translate(glm::mat4{ 1.0f }, translate) * glm::scale(glm::mat4{ 1.0f }, scale);
				Renderer2D::DrawCircle(transform, glm::vec4{ 0.1f, 0.9f, 0.1f, 1.0f }, cc2D_comp.Radius, 0.025f);
			}
		}


		Renderer2D::EndScene();
	}

	std::pair<glm::vec3, glm::vec3> EditorLayer::RayCast(const EditorCamera& camera, float mx, float my)
	{
		glm::vec4 mouse_clip_position = { mx, my, -1.0f, 1.0f };

		auto inverse_projection = glm::inverse(camera.GetProjection());
		auto inverse_view		= glm::inverse(glm::mat3{ camera.GetViewMatrix() });
		glm::vec4 ray			= inverse_projection * mouse_clip_position;
		glm::vec3 ray_pos		= camera.GetTranslation();
		glm::vec3 ray_dir		= inverse_view * glm::vec3{ ray };

		return { ray_pos, ray_dir };
	}
}
