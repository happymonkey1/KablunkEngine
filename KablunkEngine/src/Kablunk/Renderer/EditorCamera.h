#ifndef KABLUNK_RENDERER_EDITOR_CAMERA_H
#define KABLUNK_RENDERER_EDITOR_CAMERA_H

#include "Camera.h"
#include "Kablunk/Core/Timestep.h"
#include "Kablunk/Events/Event.h"
#include "Kablunk/Events/MouseEvent.h"
#include "Kablunk/Core/KeyCodes.h"

#include <glm/glm.hpp>

namespace Kablunk
{
	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspect_ratio, float near_clip, float far_clip);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		float GetDistance() const { return m_distance; }
		void SetDistance(float distance) { m_distance = distance; }

		void OnViewportResize(float width, float height)
		{
			m_viewport_width  = width;
			m_viewport_height = height;
			UpdateProjection();
		}

		const glm::mat4& GetViewMatrix() const { return m_view_matrix; }
		glm::mat4 GetViewProjectionMatrix() const { return m_projection * m_view_matrix; }

		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		const glm::vec3& GetTranslation() const { return m_translation; }
		glm::quat GetOrientation() const;

		float GetPitch() const { return m_pitch; }
		float GetYaw() const { return m_yaw; }

		static constexpr int Camera_control_key = Key::LeftAlt;
	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculateTranslation() const;
		std::pair<float, float> GetPanSpeed() const;
		constexpr float GetRotationSpeed() const { return 0.8f; };
		float GetZoomSpeed() const;
	private:
		float m_fov = 45.0f, m_aspect_ratio = 1.778f, m_near_clip = 0.1f, m_far_clip = 1000.0f;

		glm::mat4 m_view_matrix;
		glm::vec3 m_translation{ 0.0f };
		glm::vec3 m_focal_point{ 0.0f };

		glm::vec2 m_initial_mouse_position{ 0.0f };

		float m_distance{ 10.0f };
		float m_pitch{ 0.0f }, m_yaw{ 0.0f };
		float m_viewport_width = 1600, m_viewport_height = 900;
	};
}

#endif

