#include <kablunk.h>
#include <Kablunk/Core/EntryPoint.h>

#include "Sandbox2D.h"
#include "FallingSand/FallingSand.h"
#include "AStarPathfinding/Pathfinding2D.h"


//#include "imgui.h"
class ExampleLayer : public Kablunk::Layer {
public:
	ExampleLayer()
		: Layer("Example")
	{

		
	}

	~ExampleLayer()
	{

	}


	void OnUpdate(Kablunk::Timestep ts) override {
		
	}

	void OnImGuiRender(Kablunk::Timestep ts) override 
	{
		
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
		}

	}

	void OnEvent(Kablunk::Event& e) {
		
	}
private:
	float m_ImguiUpdateCounter = 0.0f;
	float m_ImguiUpdateCounterMax = .1f;
	float m_ImguiDeltaTime = 10.0f;
	float m_ImguiFPS = 10.0f;
};


class Sandbox : public Kablunk::Application {
public:
	Sandbox() {
		PushLayer(new ExampleLayer());
		//PushLayer(new Sandbox2D());
		PushLayer(new FallingSand());
		//PushLayer(new PathfindingLayer());

		SetWindowTitle("Sandbox 2D");
	}

	~Sandbox() {

	}

private:
	
};

Kablunk::Application* Kablunk::CreateApplication() {
	return new Sandbox();
}