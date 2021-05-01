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
		if (Input::IsKeyPressed(KB_KEY_A))
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;
		else if (Input::IsKeyPressed(KB_KEY_D))
			m_CameraPosition.x += m_CameraMoveSpeed * ts;


		if (Input::IsKeyPressed(KB_KEY_W))
			m_CameraPosition.y += m_CameraMoveSpeed * ts;
		else if (Input::IsKeyPressed(KB_KEY_S))
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;


		if (Input::IsKeyPressed(KB_KEY_Q))
			m_CameraRotation += m_CameraRotationSpeed * ts;
		else if (Input::IsKeyPressed(KB_KEY_E))
			m_CameraRotation -= m_CameraRotationSpeed * ts;
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher{ e };
		dispatcher.Dispatch<MouseScrolledEvent>(KABLUNK_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(KABLUNK_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));

	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{

	}

	void OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{

	}

}