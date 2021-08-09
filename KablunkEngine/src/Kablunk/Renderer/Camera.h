#ifndef KABLUNK_RENDERER_CAMERA_H
#define KABLUNK_RENDERER_CAMERA_H

#include <glm/glm.hpp>

namespace Kablunk
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection) : m_projection{ projection } { }
		virtual ~Camera() = default;

		const glm::mat4& GetProjection() const { return m_projection; }
		
		// #TODO cleanup function for appropriate API
		glm::vec2 ScreenToWorldPosition2D(const glm::mat4& view, const glm::vec2& mouse_pos) const
		{
			float viewport_width = 1.0f, viewport_height = 1.0f;
			float depth_value = 1.0f;
			auto inverse_mat = m_projection * glm::inverse(view);
			
			auto scaled_mouse = glm::vec4{
				mouse_pos.x / viewport_width,
				mouse_pos.y / viewport_height,
				depth_value,
				1.0f
			};

			// adjust for perspective
			auto result = scaled_mouse * inverse_mat;
			result = result / result.w;
			return result;
		}
	protected:
		glm::mat4 m_projection{ 1.0f };
	};
}

#endif
