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
	private:
		glm::mat4 m_projection;
	};
}
#endif
