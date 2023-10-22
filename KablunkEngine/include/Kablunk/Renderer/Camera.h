#ifndef KABLUNK_RENDERER_CAMERA_H
#define KABLUNK_RENDERER_CAMERA_H

#include "Kablunk/Core/Input.h"
#include <glm/glm.hpp>

namespace kb
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection, const glm::mat4& unreversed_projection) : m_projection{ projection }, 
			m_unreversed_projection{ unreversed_projection } { }
		virtual ~Camera() = default;

		const glm::mat4& GetProjection() const { return m_projection; }
		const glm::mat4& GetUnreversedProjection() const { return m_unreversed_projection; }
	protected:
		glm::mat4 m_projection{ 1.0f };
		// Needed for ImGuizmo
		glm::mat4 m_unreversed_projection{ 1.0f };
	};
}

#endif
