#include "kablunkpch.h"
#include "Kablunk/Core/OrthographicCameraController.h"
#include "Kablunk/Core/Input.h"


namespace Kablunk
{

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_AspectRatio{ aspectRatio }, m_ZoomLevel{ 1.0f }, 
		  m_Camera{ -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel }, m_Rotation{rotation}
	{

	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		KB_PROFILE_FUNCTION();
		
		if (m_TranslationInputLocked)
			return;

		float speedMult = (Input::IsKeyPressed(KB_KEY_LEFT_SHIFT)) ? 5.0f : 1.0f;
		if (Input::IsKeyPressed(KB_KEY_A))
		{
			m_CameraPosition.x -= cos(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts * speedMult;
			m_CameraPosition.y -= sin(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts * speedMult;
		}
		else if (Input::IsKeyPressed(KB_KEY_D))
		{
			m_CameraPosition.x += cos(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts * speedMult;
			m_CameraPosition.y += sin(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts * speedMult;
		}

		if (Input::IsKeyPressed(KB_KEY_W))
		{
			m_CameraPosition.x += -sin(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts * speedMult;
			m_CameraPosition.y +=  cos(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts * speedMult;
		}
		else if (Input::IsKeyPressed(KB_KEY_S))
		{
			m_CameraPosition.x -= -sin(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts * speedMult;
			m_CameraPosition.y -= cos(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts * speedMult;
		}
		
		
		if (m_Rotation)
		{
			if (Input::IsKeyPressed(KB_KEY_Q))
				m_CameraRotation += m_CameraRotationSpeed * ts * speedMult;
			else if (Input::IsKeyPressed(KB_KEY_E))			  
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
		KB_PROFILE_FUNCTION();

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

		KB_PROFILE_FUNCTION();
		float speedMult = (Input::IsKeyPressed(KB_KEY_LEFT_SHIFT)) ? 5.0f : 1.0f;
		m_ZoomLevel -= e.GetYOffset() * 0.25f * speedMult;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);

		
		m_CameraMoveSpeed = m_ZoomLevel * 0.5f;
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		KB_PROFILE_FUNCTION();

		OnResize((float)e.GetWidth(), (float)e.GetHeight());
		return false;
	}

}