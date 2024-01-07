#include "kablunkpch.h"
#include "Kablunk/Core/OrthographicCameraController.h"
#include "Kablunk/Core/Input.h"


namespace kb
{
OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
	: m_AspectRatio{ aspectRatio }, m_ZoomLevel{ 1.0f }, 
	  m_Camera{ -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel }, m_Rotation{rotation}
{
}

void OrthographicCameraController::OnUpdate(Timestep ts)
{
    KB_PROFILE_SCOPE;

	if (m_TranslationInputLocked)
		return;

	float speedMult = (input::is_key_pressed(Key::LeftShift)) ? 5.0f : 1.0f;
	if (input::is_key_pressed(Key::A))
	{
		m_CameraPosition.x -= cos(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts * speedMult;
		m_CameraPosition.y -= sin(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts * speedMult;
	}
	else if (input::is_key_pressed(Key::D))
	{
		m_CameraPosition.x += cos(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts * speedMult;
		m_CameraPosition.y += sin(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts * speedMult;
	}

	if (input::is_key_pressed(Key::W))
	{
		m_CameraPosition.x += -sin(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts * speedMult;
		m_CameraPosition.y +=  cos(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts * speedMult;
	}
	else if (input::is_key_pressed(Key::D))
	{
		m_CameraPosition.x -= -sin(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts * speedMult;
		m_CameraPosition.y -= cos(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts * speedMult;
	}

	if (m_Rotation)
	{
		if (input::is_key_pressed(Key::Q))
			m_CameraRotation += m_CameraRotationSpeed * ts * speedMult;
		else if (input::is_key_pressed(Key::E))
			m_CameraRotation -= m_CameraRotationSpeed * ts * speedMult;

		if (m_CameraRotation > 180.0f)
			m_CameraRotation -= 360.0f;
		else if (m_CameraRotation <= -180.0f)
			m_CameraRotation += 360.0f;

		m_Camera.SetRotation(m_CameraRotation);
	}

	m_Camera.SetPosition(m_CameraPosition);
}

void OrthographicCameraController::OnEvent(Event& e)
{
    KB_PROFILE_SCOPE;

	EventDispatcher dispatcher{ e };
	dispatcher.Dispatch<MouseScrolledEvent>(KABLUNK_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
	dispatcher.Dispatch<WindowResizeEvent>(KABLUNK_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
}

void OrthographicCameraController::OnResize(float width, float height)
{
	m_AspectRatio = width / height;
	m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
}

bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
{
	if (m_ScalingInputLocked)
		return false;

    KB_PROFILE_SCOPE;
	float speedMult = (input::is_key_pressed(Key::LeftShift)) ? 5.0f : 1.0f;
	m_ZoomLevel -= e.GetYOffset() * 0.25f * speedMult;
	m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
	m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);

	m_CameraMoveSpeed = m_ZoomLevel * 0.5f;
	return false;
}

bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
{
    KB_PROFILE_SCOPE;

	OnResize((float)e.GetWidth(), (float)e.GetHeight());
	return false;
}
}
