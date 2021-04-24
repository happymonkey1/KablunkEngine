#include <kablunkpch.h>
#include <kablunk.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include "imgui.h"
class GameLayer : public Kablunk::Layer {
public:
	GameLayer()
		: Layer("Game"), m_OrthographicCamera{ -1.6f, 1.6f, -0.9f, 0.9f }
	{

	}

	~GameLayer()
	{

	}


	void OnUpdate(Kablunk::Timestep ts) override {
		Kablunk::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Kablunk::RenderCommand::Clear();

		Kablunk::Renderer::BeginScene(m_OrthographicCamera);

		Kablunk::Renderer::EndScene();
	}

	void OnImGuiRender(Kablunk::Timestep ts) override {
		if (m_ImguiUpdateCounter >= m_ImguiUpdateCounterMax)
		{
			float miliseconds = ts.GetMiliseconds();
			m_ImguiDeltaTime = miliseconds;
			m_ImguiFPS = 1000.0f / miliseconds;
			m_ImguiUpdateCounter -= m_ImguiUpdateCounterMax;
		}
		else
			m_ImguiUpdateCounter += ts.GetMiliseconds() / 1000.0f;

		ImGui::Begin("Debug Information");

		ImGui::Text("Frame time: %.*f", 4, m_ImguiDeltaTime);
		ImGui::Text("FPS: %.*f", 4, m_ImguiFPS);

		ImGui::End();
	}

	void OnEvent(Kablunk::Event& e) {
		
	}

private:
	Kablunk::OrthographicCamera m_OrthographicCamera;

	float m_ImguiUpdateCounter = 0.0f;
	float m_ImguiUpdateCounterMax = .1f;
	float m_ImguiDeltaTime = 10.0f;
	float m_ImguiFPS = 10.0f;
};


class FallingSandSanbox : public Kablunk::Application {
public:
	FallingSandSanbox() {
		PushLayer(new GameLayer());


	}

	~FallingSandSanbox() {

	}

private:

};

Kablunk::Application* Kablunk::CreateApplication() {
	return new FallingSandSanbox();
}