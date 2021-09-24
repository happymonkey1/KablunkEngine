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

		void OnResize(float width, float height);

		OrthographicCamera& GetCamera() { return m_Camera; };
		const OrthographicCamera& GetCamera() const { return m_Camera; };

		void SetZoomLevel(float level) {
			m_ZoomLevel = level; 
			m_CameraMoveSpeed = m_ZoomLevel * 0.5f;
			m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		};
		float GetZoomLevel() { return m_ZoomLevel; };

		void SetPosition(const glm::vec3& pos) { m_CameraPosition = pos; m_Camera.SetPosition(m_CameraPosition); }
		const glm::vec3& GetPosition() const { return m_CameraPosition; }

		float GetAspectRatio() const { return m_AspectRatio; }

		void SetTranslationInputLocked(bool locked) { m_TranslationInputLocked = locked; }
		bool GetTranslationInputLocked() { return m_TranslationInputLocked; }

		void SetScalingInputLocked(bool locked) { m_ScalingInputLocked = locked; }
		bool GetScalingInputLocked() { return m_ScalingInputLocked; }
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

		bool m_TranslationInputLocked{ false };
		bool m_ScalingInputLocked{ false };
	};
}

#endif