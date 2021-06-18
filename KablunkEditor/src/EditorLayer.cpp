#include "EditorLayer.h"

#include <glm/gtc/type_ptr.hpp>


namespace Kablunk
{

	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_camera_controller{ 1.7778f, true }
	{
		ImGuiIO& io = ImGui::GetIO();
		m_imgui_font = io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Medium.ttf", 16);
		io.Fonts->Build();

		m_active_scene = CreateRef<Scene>();

		auto square = m_active_scene->CreateEntity("Square Entity");

		square.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f });
	
		m_square_entity = square;
	}

	void EditorLayer::OnAttach()
	{
		m_missing_texture	= Texture2D::Create("assets/textures/missing_texture.png");
		m_kablunk_logo		= Texture2D::Create("assets/textures/kablunk_logo.png");
		m_icon_play			= Texture2D::Create("assets/icons/round_play_arrow_white_72dp.png");

		FrameBufferSpecification frame_buffer_specs;
		frame_buffer_specs.width = 1280;
		frame_buffer_specs.height = 720;
		m_frame_buffer = Framebuffer::Create(frame_buffer_specs);
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

		if (m_ImguiUpdateCounter >= m_ImguiUpdateCounterMax)
		{
			float miliseconds = ts.GetMiliseconds();
			m_ImguiDeltaTime = miliseconds;
			m_ImguiFPS = 1000.0f / miliseconds;
			m_ImguiUpdateCounter -= m_ImguiUpdateCounterMax;
		}
		else
			m_ImguiUpdateCounter += ts.GetMiliseconds() / 1000.0f;

		
		// ==========
		//   Render
		// ==========

		m_frame_buffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();


		Renderer2D::ResetStats();
		Renderer2D::BeginScene(m_camera_controller.GetCamera());

		m_active_scene->OnUpdate(ts);
			
		Renderer2D::EndScene();

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
		ImGui::PushFont(m_imgui_font);

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
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("##kablunk_editor_dockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}



		float main_menu_window_height{ -1.0f };
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::MenuItem("Open");

				if (ImGui::MenuItem("Exit")) Application::Get().Close();

				ImGui::EndMenu();
			}

			main_menu_window_height = ImGui::GetFrameHeight();
			ImGui::EndMenuBar();
		}
		
		
		//ImGui::PopStyleVar();

		//ImGui::Dummy({ 0.0f, 200.0f });

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

		
		ImGui::Begin("Debug Information");
		
		ImGui::Text("Frame time: %.*f", 4, m_ImguiDeltaTime);
		ImGui::Text("FPS: %.*f", 4, m_ImguiFPS);

		Kablunk::Renderer2D::Renderer2DStats stats = Kablunk::Renderer2D::GetStats();

		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Verts: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		ImGui::Text("Quad Count: %d", stats.QuadCount);
		
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
		ImGui::Begin("Viewport");
		
		m_viewport_focused = ImGui::IsWindowFocused();
		m_viewport_hovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->SetAllowEventPassing(m_viewport_focused&& m_viewport_hovered);

		auto panel_size = ImGui::GetContentRegionAvail();
		if (m_viewport_size != *((glm::vec2*)&panel_size))
		{
			auto width = panel_size.x, height = panel_size.y;
			m_frame_buffer->Resize(width, height);
			m_viewport_size = { width, height };

			m_camera_controller.OnResize(width, height);
		}

		auto frame_buffer_id = m_frame_buffer->GetColorAttachmentRendererID();
		ImGui::Image(reinterpret_cast<void*>(frame_buffer_id), { m_viewport_size.x, m_viewport_size.y }, { 0.0f, 1.0f }, { 1.0f, 0.0f });
		
		ImGui::PopStyleVar();
		ImGui::End();

		ImGui::PopFont();
		ImGui::End();
		
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_camera_controller.OnEvent(e);

		if (e.GetEventType() == EventType::WindowMinimized)
		{
			m_viewport_size = { 0.0f, 0.0f };
		}
	}

}