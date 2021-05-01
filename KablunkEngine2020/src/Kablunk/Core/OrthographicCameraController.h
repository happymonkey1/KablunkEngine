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

		OrthographicCamera& GetCamera() { return m_Camera; };
		const OrthographicCamera& GetCamera() const { return m_Camera; };

		void SetZoomLevel(float level) { m_ZoomLevel = level; };
		float GetZoomLevel() { return m_ZoomLevel; };
	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);
	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		OrthographicCamera m_Camera;

		bool m_Rotation;
		glm::vec3 m_CameraPosition{ 0.0f, 0.0f, 0.0f };
		float m_CameraRotation{ 0.0f };

		float m_CameraMoveSpeed{ 1.0f };
		float m_CameraRotationSpeed{ 180.0f };
	};
}

#endif