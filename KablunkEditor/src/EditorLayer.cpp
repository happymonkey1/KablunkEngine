#include "EditorLayer.h"

#include "Kablunk/Utilities/PlatformUtils.h"
#include "Kablunk/Math/Math.h"
#include "Kablunk/Core/MouseCodes.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <Kablunk/Scene/SceneSerializer.h>

#include "Kablunk/Core/Uuid64.h"

#include "ImGuizmo.h"


namespace Kablunk
{
	


	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_editor_camera{ 45.0f, 1.778f, 0.1f, 1000.0f }
	{
		m_active_scene = CreateRef<Scene>();

#if 0
		auto square = m_active_scene->CreateEntity("Square Entity");
		square.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f });

		auto red_square = m_active_scene->CreateEntity("Square Entity");
		red_square.AddComponent<SpriteRendererComponent>(glm::vec4{ 8.0f, 0.2f, 0.3f, 1.0f });
	
		m_square_entity = square;

		m_primary_camera_entity = m_active_scene->CreateEntity("Primary Camera");
		m_primary_camera_entity.AddComponent<CameraComponent>();
		m_secondary_camera_entity = m_active_scene->CreateEntity("Secondary Camera");
		auto& camera_comp = m_secondary_camera_entity.AddComponent<CameraComponent>();


		class CameraControllerScript : public ScriptableEntity
		{
		public:
			virtual void OnCreate() override
			{

			}
			
			virtual void OnUpdate(Timestep ts) override
			{
				auto& transform = GetComponent<TransformComponent>();
				static float speed = 10.0f;

				if (Input::IsKeyPressed(Key::W))			transform.Translation.y += speed * ts;
				else if (Input::IsKeyPressed(Key::S))		transform.Translation.y -= speed * ts;
				if (Input::IsKeyPressed(Key::A))			transform.Translation.x -= speed * ts;
				else if (Input::IsKeyPressed(Key::D))		transform.Translation.x += speed * ts;
			}
		};

		m_primary_camera_entity.AddComponent<NativeScriptComponent>().Bind<CameraControllerScript>();

		camera_comp.Primary = false;

		auto dummy_child_test_ent = m_active_scene->CreateEntity("Dummy Child");
		m_primary_camera_entity.AddChild(dummy_child_test_ent);
#endif
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

		m_hierarchy_panel.SetContext(m_active_scene);

		/*for (int i = 0; i < 20; ++i)
		{
			auto test_uuid = uuid::generate();
			KB_CORE_INFO("{0}", uuid::to_string(test_uuid));
		}*/
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

		m_editor_camera.OnUpdate(ts);

		if (m_imgui_profiler_stats.Counter >= m_imgui_profiler_stats.Counter_max)
		{
			float miliseconds = ts.GetMiliseconds();
			m_imgui_profiler_stats.Delta_time = miliseconds;
			m_imgui_profiler_stats.Fps = 1000.0f / miliseconds;
			m_imgui_profiler_stats.Counter -= m_imgui_profiler_stats.Counter_max;
		}
		else
			m_imgui_profiler_stats.Counter += ts.GetMiliseconds() / 1000.0f;

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

		m_frame_buffer->Bind();

		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		// Clear our entity ID buffer to -1
		m_frame_buffer->ClearAttachment(1, -1);

		m_active_scene->OnUpdateEditor(ts, m_editor_camera);

		ViewportClickSelectEntity();

		m_frame_buffer->Unbind();
	}

	void EditorLayer::OnImGuiRender(Timestep ts)
	{
		KB_PROFILE_FUNCTION();

		static bool dockspace_open	= true;
		static bool opt_fullscreen	= true;
		static bool opt_padding		= false;
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
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}


		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("##kablunk_editor_dockspace", NULL, window_flags);

		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		

		/*auto secondary_toolbar_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove;
		static float padding = 10.0f;
		if (ImGui::Begin("##secondary_toolbar", NULL, secondary_toolbar_flags))
		{
			auto play_button_icon_id = m_icon_play->GetRendererID();
			ImGui::ImageButton((void*)play_button_icon_id, { 30.0f, 30.0f });


			ImGui::End();
		}*/

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		auto& style = ImGui::GetStyle();
		float min_window_size = style.WindowMinSize.x;
		style.WindowMinSize.x = 375.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("##kablunk_editor_dockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = min_window_size;



		float main_menu_window_height{ -1.0f };
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Crtl+N"))
					NewScene();
				

				if (ImGui::MenuItem("Open...", "Crtl+O"))
					OpenScene();
				

				if (ImGui::MenuItem("Save As...", "Crtl+Shift+S"))
					SaveSceneAs();
				

				if (ImGui::MenuItem("Exit")) 
					Application::Get().Close();

				ImGui::EndMenu();
			}

			main_menu_window_height = ImGui::GetFrameHeight();
			ImGui::EndMenuBar();
		}
		
		
		//ImGui::PopStyleVar();

		//ImGui::Dummy({ 0.0f, 200.0f });

		m_hierarchy_panel.OnImGuiRender();


		ImGui::Begin("Debug Information");
		
		ImGui::Text("Frame time: %.*f", 4, m_imgui_profiler_stats.Delta_time);
		ImGui::Text("FPS: %.*f", 4, m_imgui_profiler_stats.Fps);

		Renderer2D::Renderer2DStats stats = Kablunk::Renderer2D::GetStats();

		ImGui::Text("Draw Calls: %d", stats.Draw_calls);
		ImGui::Text("Verts: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		ImGui::Text("Quad Count: %d", stats.Quad_count);
		
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
		ImGui::Begin("Viewport");
		auto viewport_min_region	= ImGui::GetWindowContentRegionMin();
		auto viewport_max_region	= ImGui::GetWindowContentRegionMax();
		auto viewport_offset		= ImGui::GetWindowPos();
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


		// ImGuizmo

		// #TODO refactor to use callbacks instead of querying current scene
		auto selected_entity = m_hierarchy_panel.GetSelectedEntity();
		if (selected_entity && m_gizmo_type != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			float window_width  = m_viewport_bounds[1].x - m_viewport_bounds[0].x;
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
			const auto& camera_projection	= m_editor_camera.GetProjection();
			auto camera_view				= m_editor_camera.GetViewMatrix();

			// Selected Entity Transform
			auto& transform_component = selected_entity.GetComponent<TransformComponent>();
			auto transform = transform_component.GetTransform();

			// Snapping
			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snap_value = (m_gizmo_type == ImGuizmo::ROTATE) ? 45.0f : 0.5f;

			float snap_values[3] = { snap_value, snap_value, snap_value };

			ImGuizmo::Manipulate(glm::value_ptr(camera_view), glm::value_ptr(camera_projection), static_cast<ImGuizmo::OPERATION>(m_gizmo_type),
				ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr, snap ? snap_values : nullptr);


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

		
		ImGui::PopStyleVar();
		ImGui::End();

		ImGui::End();
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

		bool ctrl_pressed  = Input::IsKeyPressed(Key::LeftControl) | Input::IsKeyPressed(Key::RightControl);
		bool shift_pressed = Input::IsKeyPressed(Key::LeftShift)   | Input::IsKeyPressed(Key::RightShift);

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
				OpenScene();

			break;
		}
		case Key::S:
		{
			if (ctrl_pressed && shift_pressed)
				SaveSceneAs();

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
			m_hierarchy_panel.SetSelectionContext(m_selected_entity);

		return true;
	}

	void EditorLayer::NewScene()
	{
		m_active_scene = CreateRef<Scene>();
		m_active_scene->OnViewportResize(static_cast<uint32_t>(m_viewport_size.x), static_cast<uint32_t>(m_viewport_size.y));
		m_hierarchy_panel.SetContext(m_active_scene);
	}

	void EditorLayer::SaveSceneAs()
	{
		auto filepath = FileDialog::SaveFile("Kablunk Scene (*.kablunkscene)\0*.kablunkscene\0");
		if (!filepath.empty())
		{
			auto serializer = SceneSerializer{ m_active_scene };
			serializer.Serialize(filepath);
		}
	}

	void EditorLayer::OpenScene()
	{
		auto filepath = FileDialog::OpenFile("Kablunk Scene (*.kablunkscene)\0*.kablunkscene\0");
		if (!filepath.empty())
		{
			NewScene();

			auto serializer = SceneSerializer{ m_active_scene };
			serializer.Deserialize(filepath);
		}
	}

	// #TODO Currently streams a second full viewport width and height framebuffer from GPU to use for mousepicking.
	//		 Consider refactoring to only stream a 3x3 framebuffer around the mouse click to save on bandwidth 
	void EditorLayer::ViewportClickSelectEntity()
	{
		// Don't check for viewport entity selection if we are not hovering
		if (!m_viewport_hovered) return;
		if (Input::IsKeyPressed(EditorCamera::Camera_control_key) || ImGuizmo::IsOver()) return;

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
}
