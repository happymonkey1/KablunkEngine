#include "kablunkpch.h"
#include "Kablunk/Scene/SceneCamera.h"
#include <glm/gtc/matrix_transform.hpp>

#include "Kablunk/Core/Application.h"

#include "Kablunk/Imgui/ImGuiGlobalContext.h"

#include "Kablunk/Renderer/RenderCommand.h"

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
		if (width == 0 || height == 0)
		{
			KB_CORE_ERROR("[SceneCamera]: Trying to set aspect ratio with width={}, height={}!", width, height);
			return;
		}

		m_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
		RecalculateProjection();
	}

	glm::vec3 SceneCamera::ScreenToWorldPoint(const glm::vec3& screen_pos, const glm::mat4& transform) const
	{
		glm::vec2 mouse_pos = glm::vec2{ 0.0f };
		glm::vec2 window_pos = glm::vec2{ 0.0f };
		glm::vec2 window_size;

		if (Kablunk::Application::Get().GetSpecification().Enable_imgui)
		{
			/*ImGuiContext* imgui_context = ImGuiGlobalContext::get().get_context();
			KB_CORE_ASSERT(imgui_context, "[SceneCamera] imgui global context not found!");
			bool found = false;
			for (ImGuiViewport* viewport : imgui_context->Viewports)
			{
				if (!viewport->PlatformUserData)
					continue;

				window_pos = { viewport->Pos.x, viewport->Pos.y};
				window_size = { viewport->Size.x, viewport->Size.y };

				found = true;
				break;
			}

			if (!found)
			{
				KB_CORE_ERROR("SceneCamera::ScreenToWorldPoint(): ImGui enabled but could not find viewport!");
				return glm::vec3{ 0.0f };
			}*/

			// get imgui mouse pos (in screen coordinates)
			auto [x, y] = ImGui::GetMousePos();
			mouse_pos = glm::vec2{ x, y };

			// subtract viewport position (in screen coordinates)
			mouse_pos -= render::get_viewport_pos();
			window_size = render::get_viewport_size();
		}
		else
		{
			window_size = Application::Get().GetWindowDimensions();
			mouse_pos = glm::vec2{ screen_pos };
		}

		// translate to normalized device space
		float mx = (mouse_pos.x / window_size.x) * 2.0f - 1.0f;
		float my = ((mouse_pos.y / window_size.y) * 2.0f - 1.0f);

		/*KB_CORE_INFO("[SceneCamera]: viewport_pos=({}, {})", render::get_viewport_pos().x, render::get_viewport_pos().y);
		KB_CORE_INFO("[SceneCamera]: mouse_pos=({}, {})", mouse_pos.x, mouse_pos.y);
		KB_CORE_INFO("[SceneCamera]: mx,my=({}, {})", mx, my);*/

		// store homogeneous space
		// #TODO fix z
		glm::vec4 mouse_clip_pos = { mx, my, -1.0f, 1.0f };
		
		// compute projection/eye space
		glm::mat4 view_proj = GetProjection() * transform;

		// project to world space
		glm::vec4 res = glm::inverse(view_proj) * mouse_clip_pos;
		res.x /= res.w;
		res.y /= res.w;
		res.z /= res.w;

		return res;
	}

	void SceneCamera::RecalculateProjection()
	{
		switch (m_projection_type)
		{
			case ProjectionType::Perspective:
			{
				// #TODO one of these is wrong...
				m_projection = glm::perspective(m_perspective_fov, m_aspect_ratio, m_perspective_near, m_perspective_far);
				m_unreversed_projection = glm::perspective(m_perspective_fov, m_aspect_ratio, m_perspective_near, m_perspective_far);
				break;
			}
			case ProjectionType::Orthographic:
			{
				float ortho_left = -m_orthographic_size * m_aspect_ratio * 0.5f;
				float ortho_right = m_orthographic_size * m_aspect_ratio * 0.5f;
				float ortho_top = m_orthographic_size * 0.5f;
				float ortho_bottom = -m_orthographic_size * 0.5f;

				m_projection = glm::ortho(ortho_left, ortho_right, ortho_bottom, ortho_top, m_orthographic_near, m_orthographic_far);
				m_unreversed_projection = glm::ortho(ortho_left, ortho_right, ortho_bottom, ortho_top, m_orthographic_near, m_orthographic_far);
				break;
			}
		}
	}
}
