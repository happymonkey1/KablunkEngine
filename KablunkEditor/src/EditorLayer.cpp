#include "EditorLayer.h"

#include <glm/gtc/type_ptr.hpp>

namespace Kablunk
{

	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_camera_controller{ 1.7778f, true }
	{

	}

	void EditorLayer::OnAttach()
	{
		m_missing_texture = Texture2D::Create("assets/textures/missing_texture.png");
		m_kablunk_logo = Texture2D::Create("assets/textures/kablunk_logo.png");

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
		{
			KB_PROFILE_SCOPE("CameraController::Update");
			m_camera_controller.OnUpdate(ts);
		}
		// ==========
		//   Render
		// ==========
		{
			KB_PROFILE_SCOPE("Renderer Draw");
			m_frame_buffer->Bind();
			RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
			RenderCommand::Clear();


			Renderer2D::ResetStats();
			Renderer2D::BeginScene(m_camera_controller.GetCamera());

			static float rotation = 0.0f;
			rotation += ts * 50.0f;

			Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.2f, 0.3f, 0.8f, 1.0f });
			Renderer2D::DrawRotatedQuad({ -1.0f, 0.0f }, { 1.0f, 1.0f }, rotation, { 0.2f, 0.8f, 0.3f, 0.5f });
			Renderer2D::DrawRotatedQuad({ 1.0f, 0.0f }, { 1.0f, 1.0f }, 45.0f, m_missing_texture);
			Renderer2D::DrawQuad({ 0.5f, 0.5f }, { 0.5f, 0.5f }, m_square_color);
			Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_missing_texture, 10.0f);

			Renderer2D::EndScene();

			m_frame_buffer->Unbind();
		}
	}

	void EditorLayer::OnImGuiRender(Timestep ts)
	{
		KB_PROFILE_FUNCTION();

		static bool docking_enabled = true;
		if (docking_enabled)
		{
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
			ImGui::Begin("DockSpace Demo", &dockspace_open, window_flags);
			if (!opt_padding)
				ImGui::PopStyleVar();

			if (opt_fullscreen)
				ImGui::PopStyleVar(2);

			// DockSpace
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					ImGui::MenuItem("Open");

					if (ImGui::MenuItem("Exit")) Application::Get().Close();

					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			ImGui::Begin("Square Color");

			ImGui::ColorEdit4("Square", glm::value_ptr(m_square_color));
			uint32_t texture_id = m_frame_buffer->GetColorAttachmentRendererID();
			ImGui::Image((void*)texture_id, ImVec2{ 720.0f, 405.0f }, ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f });

			ImGui::End();

			ImGui::End();
		}
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_camera_controller.OnEvent(e);
	}

}