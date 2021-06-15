#ifndef PATHFINDING_MAZE_H
#define PATHFINDING_MAZE_H

#include <Kablunk.h>
#include <glm/glm.hpp>
#include <vector>
#include <functional>

struct Node
{
	static constexpr float MAX_DIFFICULTY = 100.0f;
	glm::ivec2 Position;
	float Difficulty;

	Node(int x, int y, float d) : Position{ x, y }, Difficulty{ d } { }
	Node(glm::ivec2 p, float d) : Position{ p }, Difficulty{ d } { }

	bool operator==(const Node& other) { return Position == other.Position; }
	bool operator!=(const Node& other) { return !(*this == other); }
};



class Maze
{
public:
	Maze();
	Maze(int width, int height);
	~Maze();

	void SetStartAndGoalNodes(glm::ivec2 start, glm::ivec2 end);
	void RandomStartAndGoalNodes();
	void Render(const Kablunk::OrthographicCameraController& camera);

	int32_t GetWidth() const { return m_width; }
	int32_t GetHeight() const { return m_height; }

	const Node& NodeAt(int32_t x, int32_t y) const { return m_tiles[y][x]; }
	const Node& GetStart() const { return NodeAt(m_start.x, m_start.y); }
	const Node& GetGoal() const { return NodeAt(m_goal.x, m_goal.y); }

	std::vector<Node> GetNeighbors(const Node& node)
	{
		auto neighbors = std::vector<Node>{};
		for (int y = -1; y <= 1; ++y)
		{
			for (int x = -1; x <= 1; ++x)
			{
				if (x != 0 && y != 0)
				{
					glm::vec2 p = { node.Position.x + x, node.Position.y + y };
					if (p.x >= 0 && p.x < m_width && p.y >= 0 && m_height)
						neighbors.push_back(NodeAt(p.x, p.y));
				}
			}
		}
		
		return neighbors;
	}
public:
	std::function<std::vector<Node>(const Node&)> GetNeighborsVisitor = std::bind(&Maze::GetNeighbors, this, std::placeholders::_1);

private:
	void InitTiles();


private:
	int32_t m_width, m_height;
	std::vector<std::vector<Node>> m_tiles;
	glm::ivec2 m_start{ 0, 0 }, m_goal{ 0, 0 };
};

#endif