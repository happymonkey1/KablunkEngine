#include "Sandbox2D.h"

#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController{ 1.7778f, true }
{

}

void Sandbox2D::OnAttach()
{
	m_MissingTexture = Kablunk::Texture2D::Create("assets/textures/missing_texture.png");
	m_KablunkLogo = Kablunk::Texture2D::Create("assets/textures/kablunk_logo.png");

	Kablunk::FrameBufferSpecification frame_buffer_specs;
	frame_buffer_specs.width = 1280;
	frame_buffer_specs.height = 720;
	m_frame_buffer = Kablunk::Framebuffer::Create(frame_buffer_specs);

	m_RandSeed = static_cast <unsigned> (time(0));

	for (uint32_t i = 0; i < SPRITE_COUNT; ++i)
	{
		m_SpriteColors[i] = {
				static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
				static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
				static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
				1.0f
		};
	}
}

void Sandbox2D::OnDetach()
{

}


void Sandbox2D::OnUpdate(Kablunk::Timestep ts)
{
	KB_PROFILE_FUNCTION();

	// ==========
	//   Update
	// ==========
	{
		KB_PROFILE_SCOPE("CameraController::Update");
		m_CameraController.OnUpdate(ts);
	}
	// ==========
	//   Render
	// ==========
	{
		KB_PROFILE_SCOPE("Renderer Draw");
		m_frame_buffer->Bind();
		Kablunk::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Kablunk::RenderCommand::Clear();
		

		Kablunk::Renderer2D::ResetStats();
		Kablunk::Renderer2D::BeginScene(m_CameraController.GetCamera());
		
		static float rotation = 0.0f;
		rotation += ts * 50.0f;

		Kablunk::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.2f, 0.3f, 0.8f, 1.0f });
		Kablunk::Renderer2D::DrawRotatedQuad({ -1.0f, 0.0f }, { 1.0f, 1.0f }, rotation, { 0.2f, 0.8f, 0.3f, 0.5f });
		Kablunk::Renderer2D::DrawRotatedQuad({ 1.0f, 0.0f }, { 1.0f, 1.0f }, 45.0f, m_MissingTexture);
		Kablunk::Renderer2D::DrawQuad({ 0.5f, 0.5f }, { 0.5f, 0.5f }, m_SquareColor);
		Kablunk::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_MissingTexture, 10.0f);

		Kablunk::Renderer2D::EndScene();

		Kablunk::Renderer2D::BeginScene(m_CameraController.GetCamera());
		glm::vec2 size{ 0.1f, 0.1f };
		glm::vec4 color{ 0.2f, 0.3f, 0.8f, 1.0f };
		const float kRandHigh = 50.0f;

		srand(m_RandSeed);
		float z = 0.0f;
		for (int i = 0; i < SPRITE_COUNT; ++i)
		{
			glm::vec3 pos;
			{
				KB_PROFILE_SCOPE("Math initialization - Renderer Draw");
				pos = {
					static_cast<float>(rand()) / static_cast<float>(RAND_MAX / kRandHigh),
					static_cast<float>(rand()) / static_cast<float>(RAND_MAX / kRandHigh),
					z
				};
			}

			
			Kablunk::Renderer2D::DrawQuad(pos, size, m_SpriteColors[i]);
			z += .001f;
		}
		
		Kablunk::Renderer2D::EndScene();
		m_frame_buffer->Unbind();
	}
}

void Sandbox2D::OnImGuiRender(Kablunk::Timestep ts)
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
				if (ImGui::MenuItem("Exit")) Kablunk::Application::Get().Close();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::Begin("Square Color");

		ImGui::ColorEdit4("Square", glm::value_ptr(m_SquareColor));
		uint32_t texture_id = m_frame_buffer->GetColorAttachmentRendererID();
		ImGui::Image((void*)texture_id, ImVec2{ 720.0f, 405.0f }, ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f });

		ImGui::End();

		ImGui::End();
	}
}

void Sandbox2D::OnEvent(Kablunk::Event& e)
{
	m_CameraController.OnEvent(e);
}
