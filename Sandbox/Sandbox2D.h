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

	glm::vec4 m_SquareColor{ 0.8f, 0.2f, 0.3f, 1.0f };

	Kablunk::OrthographicCameraController m_CameraController;

	Kablunk::Ref<Kablunk::Texture2D> m_MissingTexture;

	float m_ImguiUpdateCounter = 0.0f;
	float m_ImguiUpdateCounterMax = .1f;
	float m_ImguiDeltaTime = 10.0f;
	float m_ImguiFPS = 10.0f;
};

