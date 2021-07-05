#ifndef KABLUNK_RENDERER_CAMERA_H
#define KABLUNK_RENDERER_CAMERA_H

#include <glm/glm.hpp>

namespace Kablunk
{
	class Camera
	{
	public:
		Camera(const glm::mat4& projection) : m_projection{ projection } { }

		const glm::mat4& GetProjection() const { return m_projection; }
		
		glm::vec2 ScreenToWorldPosition2D(const glm::mat4& view, const glm::vec2& mouse_pos) const
		{
			auto inverse_mat = glm::inverse(m_projection * view);
			auto scaled_mouse_position = glm::vec4{ mouse_pos.x, mouse_pos.y, 1.0f, 1.0f };
			auto out_coords = scaled_mouse_position * inverse_mat;
			out_coords.x /= out_coords.w;
			out_coords.y /= out_coords.w;
			out_coords.z /= out_coords.w;
			return out_coords;
		}
	private:
		glm::mat4 m_projection;
	};
}

#endif
