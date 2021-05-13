#ifndef FALLINGSAND_ENTITIES_H
#define FALLINGSAND_ENTITIES_H

#include <Kablunk.h>
#include <glm/glm.hpp>

class Entity
{
public:
	Entity(const glm::vec2& p, const glm::vec2& v) : m_Position{ p.x, p.y, 1.0f }, m_Velocity{ v } { }
	Entity(const glm::vec3& p, const glm::vec2& v) : m_Position{ p }, m_Velocity{ v } { }

	virtual void OnUpdate(Kablunk::Timestep ts)  = 0;
	virtual void OnRender() = 0;
protected:
	glm::vec3 m_Position;
	glm::vec2 m_Velocity;
};


class Frog : public Entity
{
public:
	Frog() = delete;
	Frog(const glm::vec2& p, const glm::vec2& s, const glm::vec2& v);
	Frog(const glm::vec3& p, const glm::vec2& s, const glm::vec2& v);

	virtual void OnUpdate(Kablunk::Timestep ts) override;
	virtual void OnRender() override;

private:
	Kablunk::Ref<Kablunk::Texture2D> m_Texture;
	glm::vec2 m_Size;
};

#endif