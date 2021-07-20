#include "EditorLayer.h"

#include "Kablunk/Utilities/PlatformUtils.h"

#include <glm/gtc/type_ptr.hpp>

#include <Kablunk/Scene/SceneSerializer.h>

namespace Kablunk
{
	


	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_camera_controller{ 1.7778f, true }
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
		m_missing_texture	= Texture2D::Create("assets/textures/missing_texture.png");
		m_kablunk_logo		= AssetManager::Create<Texture2D>("assets/textures/kablunk_logo.png");
		m_icon_play			= Texture2D::Create("assets/icons/round_play_arrow_white_72dp.png");

		FrameBufferSpecification frame_buffer_specs;
		auto window_dimensions = Application::Get().GetWindowDimensions();
		frame_buffer_specs.width  = window_dimensions.x;
		frame_buffer_specs.height = window_dimensions.y;
		m_frame_buffer = Framebuffer::Create(frame_buffer_specs);

		m_hierarchy_panel.SetContext(m_active_scene);

		

		
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
		
		if (m_viewport_focused) m_camera_controller.OnUpdate(ts);

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
			&& (spec.width != m_viewport_size.x || spec.height != m_viewport_size.y))
		{
			m_frame_buffer->Resize(static_cast<uint32_t>(m_viewport_size.x), static_cast<uint32_t>(m_viewport_size.y));
			m_camera_controller.OnResize(m_viewport_size.x, m_viewport_size.y);

			m_active_scene->OnViewportResize(static_cast<uint32_t>(m_viewport_size.x), static_cast<uint32_t>(m_viewport_size.y));
		}
		
		// ==========
		//   Render
		// ==========
		Renderer2D::ResetStats();

		m_frame_buffer->Bind();

		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		m_active_scene->OnUpdate(ts);

		m_frame_buffer->Unbind();
	}

	void EditorLayer::OnImGuiRender(Timestep ts)
	{
		KB_PROFILE_FUNCTION();

		static bool dockspace_open = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
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
				{
					NewScene();
				}

				if (ImGui::MenuItem("Open...", "Crtl+O"))
				{
					OpenScene();
				}

				if (ImGui::MenuItem("Save As...", "Crtl+Shift+S"))
				{
					SaveSceneAs();
				}

				if (ImGui::MenuItem("Exit")) Application::Get().Close();

				ImGui::EndMenu();
			}

			main_menu_window_height = ImGui::GetFrameHeight();
			ImGui::EndMenuBar();
		}
		
		
		//ImGui::PopStyleVar();

		//ImGui::Dummy({ 0.0f, 200.0f });

		m_hierarchy_panel.OnImGuiRender();

		/*
		if (m_square_entity)
		{
			const std::string& tag = m_square_entity.GetComponent<TagComponent>();
			ImGui::Begin(tag.c_str());

			auto& square_color = m_square_entity.GetComponent<SpriteRendererComponent>().Color;
			ImGui::ColorEdit4("Color", glm::value_ptr(square_color));
			ImGui::DragFloat2("Position", glm::value_ptr(m_square_pos), 0.1f);

			ImGui::DragFloat2("Size", glm::value_ptr(m_square_size), 0.1f);
			ImGui::DragFloat("Rotation", &m_square_rotation, 0.1f);

			ImGui::End();
		}

		if (m_primary_camera_entity)
		{
			const std::string& tag = m_primary_camera_entity.GetComponent<TagComponent>();
			ImGui::Begin(tag.c_str());
			
			auto& primary_camera_component = m_primary_camera_entity.GetComponent<CameraComponent>();
			
			if (ImGui::Checkbox("Primary Camera", &m_primary_camera_selected))
			{
				primary_camera_component.Primary = m_primary_camera_selected;
				m_secondary_camera_entity.GetComponent<CameraComponent>().Primary = !m_primary_camera_selected;
			}
			float ortho_size = primary_camera_component.Camera.GetOrthographicSize();
			if (ImGui::DragFloat("Primary Camera Size", &ortho_size, 0.25f, 1.0f, 50.0f))
				primary_camera_component.Camera.SetOrthographicSize(ortho_size);
			ImGui::End();
		}
		*/

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
		
		m_viewport_focused = ImGui::IsWindowFocused();
		m_viewport_hovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->SetAllowEventPassing(m_viewport_focused && m_viewport_hovered);

		auto panel_size = ImGui::GetContentRegionAvail();
		auto width = panel_size.x, height = panel_size.y;
		m_viewport_size = { width, height };

		auto frame_buffer_id = m_frame_buffer->GetColorAttachmentRendererID();
		ImGui::Image(
			reinterpret_cast<void*>(static_cast<uint64_t>(frame_buffer_id)), 
			{ m_viewport_size.x, m_viewport_size.y }, 
			{ 0.0f, 1.0f }, { 1.0f, 0.0f }
		);
		
		ImGui::PopStyleVar();
		ImGui::End();

		ImGui::End();
		
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_camera_controller.OnEvent(e);

		if (e.GetEventType() == EventType::WindowMinimized)
			m_viewport_size = { 0.0f, 0.0f };

		EventDispatcher dispatcher{ e };
		dispatcher.Dispatch<KeyPressedEvent>(KABLUNK_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// Shortcuts
		if (e.GetRepeatCount() > 0)
			return false;

		bool ctrl_pressed  = Input::IsKeyPressed(Key::LeftControl) | Input::IsKeyPressed(Key::RightControl);
		bool shift_pressed = Input::IsKeyPressed(Key::LeftShift) | Input::IsKeyPressed(Key::RightShift);

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
		default:
			break;
		}
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
}
