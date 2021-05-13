#include "Entities.h"
#include <glm/gtc/matrix_transform.hpp>

// ========
//   FROG
// ========

Frog::Frog(const glm::vec2& p, const glm::vec2& s, const glm::vec2& v)
	: Entity(p, v), m_Size{s}
{
	m_Texture = Kablunk::Texture2D::Create("assets/froge/entities/froge_cute_1.png");
}

Frog::Frog(const glm::vec3& p, const glm::vec2& s, const glm::vec2& v)
	: Entity(p, v), m_Size{ s }
{
	m_Texture = Kablunk::Texture2D::Create("assets/froge/entities/froge_cute_1.png");
}

void Frog::OnUpdate(Kablunk::Timestep ts)
{
	// scale velocity based on size
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), { m_Velocity.x, m_Velocity.y, 0.0f})
		* glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 0.0f });

	auto scaledVel = transform * glm::vec4{ m_Velocity, 0.0f, 0.0f };

	m_Position += glm::vec3{ scaledVel.x, scaledVel.y, 0.0f } * ts.GetMiliseconds();
}

void Frog::OnRender()
{
	Kablunk::Renderer2D::DrawQuad(m_Position, m_Size, m_Texture);
}
