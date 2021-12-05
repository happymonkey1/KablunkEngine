#ifndef KABLUNK_RENDERER_CAMERA_H
#define KABLUNK_RENDERER_CAMERA_H

#include "Kablunk/Core/Input.h"
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
	protected:
		glm::mat4 m_projection{ 1.0f };
	};
}

#endif
