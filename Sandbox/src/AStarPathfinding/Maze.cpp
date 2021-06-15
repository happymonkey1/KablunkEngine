#include <kablunk.h>
#include <random>
#include "Maze.h"



Maze::Maze()
	: m_width{ 0 }, m_height{ 0 }
{

}

Maze::Maze(int width, int height)
	: m_width{ width }, m_height{ height }
{
	InitTiles();
	RandomStartAndGoalNodes();
}

Maze::~Maze()
{

}

void Maze::SetStartAndGoalNodes(glm::ivec2 start, glm::ivec2 end)
{
	m_start = start;
	m_goal = end;
}

void Maze::RandomStartAndGoalNodes()
{
	std::random_device rd{};
	std::mt19937 generator{ rd() };

	std::uniform_int_distribution<> uniform_dist{ 0, m_width - 1 };

	m_start = glm::ivec2{ uniform_dist(generator), uniform_dist(generator) };
	m_goal = glm::ivec2{ uniform_dist(generator), uniform_dist(generator) };
}

void Maze::InitTiles()
{
	m_tiles = std::vector<std::vector<Node>>{};

	std::random_device rd{};
	std::mt19937 generator{ rd() };

	std::normal_distribution<> normal_dist{ 10, 5 };

	for (int y = 0; y < m_height; ++y)
	{
		m_tiles.push_back( std::vector<Node>{} );
		for (int x = 0; x < m_width; ++x)
		{
			m_tiles[y].push_back( Node{ x, y, static_cast<float>( normal_dist(generator) ) } );
		}
	}
}

void Maze::Render(const Kablunk::OrthographicCameraController& camera)
{
	Kablunk::Renderer2D::BeginScene(camera.GetCamera());

	for (int y = 0; y < m_height; ++y)
	{
		for (int x = 0; x < m_width; ++x)
		{
			auto tile = m_tiles[y][x];
			glm::vec4 color;
			if (tile.Position == m_start)
				color = glm::vec4{ 1.0f };
			else if (tile.Position == m_goal)
				color = glm::vec4{ 0.992f, 0.992f, 0.588f, 1.0f };
			else
				color = { 1.0f * (tile.Difficulty / Node::MAX_DIFFICULTY), 0.2f, 0.3f, 1.0f };
			
			glm::vec2 size{ 0.1f, 0.1f };
			Kablunk::Renderer2D::DrawQuad(glm::vec2{ tile.Position.x, tile.Position.y } * size, size, color);
		}
	}

	Kablunk::Renderer2D::EndScene();
}