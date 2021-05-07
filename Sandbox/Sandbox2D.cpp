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
		for (int i = 0, z = 0.0f; i < SPRITE_COUNT; ++i, z += .001f)
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
		}
		
		Kablunk::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender(Kablunk::Timestep ts)
{
	KB_PROFILE_FUNCTION();

	if (m_ImguiUpdateCounter >= m_ImguiUpdateCounterMax)
	{
		float miliseconds = ts.GetMiliseconds();
		m_ImguiDeltaTime = miliseconds;
		m_ImguiFPS = 1000.0f / miliseconds;
		m_ImguiUpdateCounter -= m_ImguiUpdateCounterMax;
	}
	else
		m_ImguiUpdateCounter += ts.GetMiliseconds() / 1000.0f;

	{
		KB_PROFILE_SCOPE("Imgui windows");
		ImGui::Begin("Debug Information");

		ImGui::Text("Frame time: %.*f", 4, m_ImguiDeltaTime);
		ImGui::Text("FPS: %.*f", 4, m_ImguiFPS);

		Kablunk::Renderer2D::Renderer2DStats stats = Kablunk::Renderer2D::GetStats();

		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Verts: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		ImGui::Text("Quad Count: %d", stats.QuadCount);

		ImGui::End();

		ImGui::Begin("Square Color");

		ImGui::ColorEdit4("Square", glm::value_ptr(m_SquareColor));

		ImGui::End();
	}
}

void Sandbox2D::OnEvent(Kablunk::Event& e)
{
	m_CameraController.OnEvent(e);
}
