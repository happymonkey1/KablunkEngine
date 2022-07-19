#include "kablunkpch.h"
#include "Kablunk/Renderer/EditorCamera.h"

#include "Kablunk/Core/Input.h"
#include "Kablunk/Core/KeyCodes.h"
#include "Kablunk/Core/MouseCodes.h"

#include <glfw/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Kablunk
{

	EditorCamera::EditorCamera(float fov, float aspect_ratio, float near_clip, float far_clip)
		: m_fov{ fov }, m_aspect_ratio{ aspect_ratio }, m_near_clip{ near_clip }, m_far_clip{ far_clip }, m_focal_point{ 0.0f },
		  Camera{ glm::perspective(glm::radians(fov), aspect_ratio, near_clip, far_clip), glm::perspective(glm::radians(fov), aspect_ratio, near_clip, far_clip) }
	{
		m_focal_point = glm::vec3{ 0.0f };

		m_position_delta = glm::vec3{ 0.0f };
		glm::vec3 position = { 0, 0, 10 };
		m_distance = glm::distance(position, m_focal_point);

		m_yaw = 0; //= 3.0f * glm::pi<float>() / 4.0f;
		m_pitch = 0; //= glm::pi<float>() / 4.0f;

		m_yaw_delta = 0.0f;
		m_pitch_delta = 0.0f;

		m_position = CalculatePosition();

		const glm::quat orientation = GetOrientation();
		m_direction = glm::eulerAngles(orientation) * (180.f / glm::pi<float>());
		glm::mat4 view_mat = glm::translate(glm::mat4(1.0f), m_position) * glm::toMat4(orientation);
		m_view_matrix = glm::inverse(view_mat);
	}

	void EditorCamera::OnUpdate(Timestep ts)
	{
		auto mouse = glm::vec2{ Input::GetMouseX(), Input::GetMouseY() };
		auto delta = (mouse - m_initial_mouse_position) * 0.002f;

		// FlyCam
		if (Input::IsMouseButtonPressed(Mouse::ButtonRight) && !Input::IsKeyPressed(Camera_control_key))
		{
			m_camera_mode = CameraMode::FlyCam;
			DisableMouse();

			const float yaw_sign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
			const float speed = 2.5f;

			if (Input::IsKeyPressed(Key::Q))
				m_position_delta -= glm::vec3{ 0.0f, yaw_sign, 0.0f } * speed * (float)ts;
			else if (Input::IsKeyPressed(Key::E))
				m_position_delta += glm::vec3{ 0.0f, yaw_sign, 0.0f } * speed * (float)ts;

			if (Input::IsKeyPressed(Key::S))
				m_position_delta -= m_direction * speed * (float)ts;
			else if (Input::IsKeyPressed(Key::W))
				m_position_delta += m_direction * speed * (float)ts;

			if (Input::IsKeyPressed(Key::A))
				m_position_delta -= m_right_direction * speed * (float)ts;
			else if (Input::IsKeyPressed(Key::D))
				m_position_delta += m_right_direction * speed * (float)ts;

			constexpr float max_rate = 0.12f;
			m_yaw_delta += glm::clamp(yaw_sign * delta.x * GetRotationSpeed(), -max_rate, max_rate);
			m_pitch_delta += glm::clamp(-delta.y * GetRotationSpeed(), -max_rate, max_rate);

			m_right_direction = glm::cross(m_direction, glm::vec3{ 0.0f, yaw_sign, 0.0f });

			m_direction = glm::rotate(
				glm::normalize(
					glm::cross(
						glm::angleAxis(-m_pitch_delta, m_right_direction),
						glm::angleAxis(-m_yaw_delta, glm::vec3{ 0.0f, yaw_sign, 0.0f })
					)
				),
				m_direction
			);

			const float distance = glm::distance(m_focal_point, m_position);
			m_focal_point = m_position + GetForwardDirection() * distance;
			m_distance = distance;
		}
		// ArcBall
		else if (Input::IsKeyPressed(Camera_control_key))
		{
			m_camera_mode = CameraMode::ArcBall;

			if (Input::IsMouseButtonPressed(Mouse::ButtonLeft)) // Left
			{
				DisableMouse();
				MousePan(delta);
			}
			else if (Input::IsMouseButtonPressed(Mouse::ButtonRight)) // Right
			{
				DisableMouse();
				MouseZoom(delta.x + delta.y);
			}
			else if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle)) // Middle
			{
				DisableMouse();
				MouseRotate(delta);
			}
			else
				EnableMouse();

		}
		else
			EnableMouse();

		m_initial_mouse_position = mouse;
		m_position += m_position_delta;
		m_yaw += m_yaw_delta;
		m_pitch += m_pitch_delta;

		if (m_camera_mode == CameraMode::ArcBall)
			m_position = CalculatePosition();

		UpdateView();
	}

	void EditorCamera::OnEvent(Event& e)
	{
		//EventDispatcher dispatcher{ e };
		//dispatcher.Dispatch<MouseScrolledEvent>(KABLUNK_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
	}

	void EditorCamera::EnableMouse() const
	{
		Input::SetCursorMode(CursorMode::Normal);
	}

	void EditorCamera::DisableMouse() const
	{
		Input::SetCursorMode(CursorMode::Locked);
	}

	void EditorCamera::UpdateProjection()
	{
		m_aspect_ratio = m_viewport_width / m_viewport_height;
		m_projection = glm::perspective(glm::radians(m_fov), m_aspect_ratio, m_near_clip, m_far_clip);
		m_unreversed_projection = glm::perspective(glm::radians(m_fov), m_aspect_ratio, m_near_clip, m_far_clip);
	}

	void EditorCamera::UpdateView()
	{
		const float yaw_sign = GetUpDirection().y < 0 ? -1.0f : 1.0f;

		//handle when camera direction is same as up direction
		const float cos_angle = glm::dot(GetForwardDirection(), GetUpDirection());
		if (cos_angle * yaw_sign > 0.99f)
			m_pitch_delta = 0.0f;

		const glm::vec3 look_at = m_position + GetForwardDirection();
		m_direction = glm::normalize(look_at - m_position);
		m_distance = glm::distance(m_position, m_focal_point);
		m_view_matrix = glm::lookAt(m_position, look_at, glm::vec3{ 0.0f, yaw_sign, 0.0f }); //* glm::toMat4(GetOrientation());
		//m_translation = CalculateTranslation();

		//auto orientation = GetOrientation();
		//m_view_matrix = glm::translate(glm::mat4{ 1.0f }, m_position) * glm::toMat4(orientation);
		//m_view_matrix = glm::inverse(m_view_matrix);

		m_yaw_delta *= 0.6f;
		m_pitch_delta *= 0.6f;
		m_position_delta *= 0.8f;
	}

	void EditorCamera::Focus(const glm::vec3& focus_point)
	{
		m_focal_point = focus_point;
		m_camera_mode = CameraMode::FlyCam;

		if (m_distance > m_min_focus_distance)
		{
			m_distance -= m_distance - m_min_focus_distance;
			m_position = m_focal_point - GetForwardDirection() * m_distance;
		}

		m_position = m_focal_point - GetForwardDirection() * m_distance;
		UpdateView();
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		float delta = e.GetYOffset() * 0.1f;
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [x_speed, y_speed] = GetPanSpeed();
		m_focal_point -= GetRightDirection() * delta.x * x_speed * m_distance;
		m_focal_point += GetUpDirection() * delta.y * y_speed * m_distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta)
	{
		float yaw_sign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		auto rotation_speed = GetRotationSpeed();
		m_yaw_delta += yaw_sign * delta.x * rotation_speed;
		m_pitch_delta += delta.y * rotation_speed;
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_distance -= delta * GetZoomSpeed();
		const glm::vec3 forward_dir = GetForwardDirection();
		m_position = m_focal_point - forward_dir * m_distance;
		if (m_distance < 1.0f)
		{
			m_focal_point += forward_dir * m_distance;
			m_distance = 1.0f;
		}

		m_position_delta += delta * GetZoomSpeed() * forward_dir;
	}

	std::pair<float, float> EditorCamera::GetPanSpeed() const
	{
		float x = std::min(m_viewport_width / 1000.0f, 2.4f);
		float x_factor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_viewport_height / 1000.0f, 2.4f);
		float y_factor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { x_factor, y_factor };
	}

	float EditorCamera::GetZoomSpeed() const
	{
		float distance	= std::max( m_distance * 0.2f, 0.0f );
		float speed		= std::min( distance * distance, 100.0f );
		
		return speed;
	}

	glm::vec3 EditorCamera::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3{ 0.0f, 1.0f, 0.0f });
	}

	glm::vec3 EditorCamera::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3{ 1.0f, 0.0f, 0.0f });
	}

	glm::vec3 EditorCamera::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3{ 0.0f, 0.0f, -1.0f });
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3{ -m_pitch - m_pitch_delta, -m_yaw - m_yaw_delta, 0.0f });
	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_focal_point - GetForwardDirection() * m_distance + m_position_delta;
	}

}
