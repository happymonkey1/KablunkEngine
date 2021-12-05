#include "kablunkpch.h"
#include "Kablunk/Scene/SceneCamera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Kablunk
{
	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}

	void SceneCamera::SetPerspective(float vertical_fov, float near_clip, float far_clip)
	{
		m_projection_type = ProjectionType::Perspective;

		m_perspective_far = vertical_fov;
		m_perspective_near = near_clip;
		m_perspective_far = far_clip;

		RecalculateProjection();
	}

	void SceneCamera::SetProjectionType(ProjectionType new_projection_type)
	{
		bool perspective_changed = new_projection_type != m_projection_type;
		m_projection_type = new_projection_type; 

		if (perspective_changed)
			RecalculateProjection();
	}

	void SceneCamera::SetOrthographic(float size, float near_clip, float far_clip)
	{
		m_projection_type = ProjectionType::Orthographic;

		m_orthographic_size = size;
		m_orthographic_near = near_clip;
		m_orthographic_far = far_clip;

		RecalculateProjection();
	}

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
		RecalculateProjection();
	}

	void SceneCamera::RecalculateProjection()
	{
		switch (m_projection_type)
		{
			case ProjectionType::Perspective:
			{
				m_projection = glm::perspective(m_perspective_fov, m_aspect_ratio, m_perspective_near, m_perspective_far);

				break;
			}
			case ProjectionType::Orthographic:
			{
				float ortho_left = -m_orthographic_size * m_aspect_ratio * 0.5f;
				float ortho_right = m_orthographic_size * m_aspect_ratio * 0.5f;
				float ortho_top = m_orthographic_size * 0.5f;
				float ortho_bottom = -m_orthographic_size * 0.5f;

				m_projection = glm::ortho(ortho_left, ortho_right, ortho_bottom, ortho_top, m_orthographic_near, m_orthographic_far);

				break;
			}
		}
	}
}
