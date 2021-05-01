#pragma once
#include <Kablunk.h>


class Sandbox2D : public Kablunk::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Kablunk::Timestep ts) override;
	virtual void OnImGuiRender(Kablunk::Timestep ts) override;
	virtual void OnEvent(Kablunk::Event& e);
private:
	Kablunk::ShaderLibrary m_ShaderLibrary;
	Kablunk::Ref<Kablunk::VertexArray> m_SquareVA;

	Kablunk::Ref<Kablunk::Texture2D> m_Texture;
	Kablunk::Ref<Kablunk::Texture2D> m_Logo;

	Kablunk::OrthographicCameraController m_CameraController;

	float m_ImguiUpdateCounter = 0.0f;
	float m_ImguiUpdateCounterMax = .1f;
	float m_ImguiDeltaTime = 10.0f;
	float m_ImguiFPS = 10.0f;
};

