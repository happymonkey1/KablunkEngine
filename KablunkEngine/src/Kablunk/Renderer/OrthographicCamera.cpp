#include "kablunkpch.h"
#include "Kablunk/Renderer/OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace kb
{

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		: m_ProjectionMatrix{glm::ortho(left, right, bottom, top, -1.0f, 1.0f)}, m_Position{glm::vec3()}, 
		m_ViewMatrix{ glm::mat4(1.0f) }, m_Rotation{ 0.0f }
	{
        KB_PROFILE_SCOPE;

		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
	{
        KB_PROFILE_SCOPE;

		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::RecalculateViewMatrix()
	{
        KB_PROFILE_SCOPE;

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) * 
			glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}
