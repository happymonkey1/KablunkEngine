#include "kablunkpch.h"
#include "Kablunk/Scene/SceneCamera.h"
#include <glm/gtc/matrix_transform.hpp>

#include "Kablunk/Core/Application.h"
// #TODO try to remove 
#include <imgui.h>
#include <imgui_internal.h>

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

	glm::vec3 SceneCamera::ScreenToWorldPoint(const glm::vec3& screen_pos, const glm::mat4& transform) const
	{
		glm::vec2 window_pos = glm::vec2{ 0.0f };
		glm::vec2 window_size;

		if (Kablunk::Application::Get().GetSpecification().Enable_imgui)
		{
			ImGuiContext* imgui_context = ImGui::GetCurrentContext();
			bool found = false;
			for (ImGuiViewport* viewport : imgui_context->Viewports)
			{
				if (!viewport->PlatformUserData)
					continue;

				window_pos = { viewport->Pos.x, viewport->Pos.y };
				window_size = { viewport->Size.x, viewport->Size.y };

				found = true;
				break;
			}

			if (!found)
			{
				KB_CORE_ERROR("SceneCamera::ScreenToWorldPoint(): ImGui enabled but could not find viewport!");
				return glm::vec3{ 0.0f };
			}
		}
		else
			window_size = Application::Get().GetWindowDimensions();


		glm::mat4 inverse = glm::inverse(GetProjection() * transform);

		// account for viewport if running scene in editor
		glm::vec3 transformed_screen_pos = { screen_pos.x - window_pos.x, screen_pos.y - window_pos.y, screen_pos.z };

		// transform position to screen space x: [-1, 1], y: [-1, 1]
		glm::vec2 screen_space_pos = {
			(2.0f * (transformed_screen_pos.x / window_size.x)) - 1.0f,
			1.0f - (2.0f * (transformed_screen_pos.y / window_size.y))
		};
		glm::vec4 in = { screen_space_pos.x, screen_space_pos.y, 0.0f, 1.0f };

		glm::vec4 res_out = in * inverse;

		
		return { res_out.x / res_out.w, res_out.y / res_out.w, res_out.z / res_out.w };
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
