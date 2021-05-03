#include "Sandbox2D.h"

#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController{ 1.7778f, true }
{

}

void Sandbox2D::OnAttach()
{
	m_MissingTexture = Kablunk::Texture2D::Create("assets/textures/missing_texture.png");
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

		Kablunk::Renderer2D::BeginScene(m_CameraController.GetCamera());
		Kablunk::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.2f, 0.3f, 0.8f, 1.0f });
		Kablunk::Renderer2D::DrawQuad({ 0.5f, 0.5f }, { 0.5f, 0.5f }, m_SquareColor);

		/*for (int y = 0; y < 100; ++y)
		{
			for (int x = 0; x < 100; ++x)
			{
				Kablunk::Renderer2D::DrawQuad({ x, y }, { 0.5f, 0.5f }, { 0.2f, 0.3f, 0.8f, 1.0f });
			}
		}*/
		//Kablunk::Renderer2D::DrawQuad({ 1.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_MissingTexture, 1.0f);
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
