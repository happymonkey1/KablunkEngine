#ifndef KABLUNK_CORE_ORTHOGRAPHIC_CAMERA_CONTROLLER
#define KABLUNK_CORE_ORTHOGRAPHIC_CAMERA_CONTROLLER

#include "Kablunk/Renderer/OrthographicCamera.h"
#include "Kablunk/Core/Timestep.h"
#include "Kablunk/Events/ApplicationEvent.h"
#include "Kablunk/Events/MouseEvent.h"

namespace Kablunk
{
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);
	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		void OnWindowResized(WindowResizeEvent& e);
	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		OrthographicCamera m_Camera;

		bool m_Rotation;
		glm::vec3 m_CameraPosition{ 0.0f, 0.0f, 0.0f };
		float m_CameraRotation{ 0.0f };

		float m_CameraMoveSpeed{ 10.0f };
		float m_CameraRotationSpeed{ 2.0f };
	};
}

#endif