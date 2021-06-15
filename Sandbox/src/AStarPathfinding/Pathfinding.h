#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "Maze.h"
#include <vector>
#include <queue>
#include <algorithm>

class Solution
{
public:
	Solution(const Maze& maze) : m_maze{ maze }, m_path{ std::vector<Node>{} } { }

	bool is_node_inside(int x, int y) const { return x >= 0 && x < m_maze.GetWidth() && y >= 0 && y < m_maze.GetHeight(); }
	
	void add_to_solution(const Node& node) { m_path.push_back(node); }
	const std::vector<Node>& get_solution() const { return m_path; }
private:
	const Maze& m_maze;
	std::vector<Node> m_path;
};

template <typename NodeType>
class Pathfinder
{
public:
	virtual ~Pathfinder() = 0;

	virtual void solve(const Maze& maze, Solution& solution) const = 0;
	virtual int evaluate(const NodeType& node) const = 0;
};

template <typename NodeType>
class DijkstraPathfinder : public Pathfinder<NodeType>
{
public:
	using NeighborFunction = std::function<std::vector<NodeType>(const NodeType&)>;
	DijkstraPathfinder(NeighborFunction neighbor_func);
	~DijkstraPathfinder() = default;

	virtual void solve(const Maze& maze, Solution& solution) const override;
	virtual int evaluate(const NodeType& node) const override;
private:
	NeighborFunction get_neighbors;
};

template <typename NodeType>
DijkstraPathfinder<NodeType>::DijkstraPathfinder(NeighborFunction neighbor_func) : get_neighbors{ neighbor_func } 
{ 

}

template <typename NodeType>
void DijkstraPathfinder<NodeType>::solve(const Maze& maze, Solution& solution) const
{
	// Add starting node to solution
	NodeType current = maze.GetStart();
	const NodeType& goal = maze.GetGoal();
	std::queue<NodeType> frontier;
	frontier.push(current);

	std::vector<NodeType> explored{};

	for (;;)
	{
		if (frontier.size() == 0) return; // Failed to find path

		current = frontier.front();
		frontier.pop();

		if (current == goal) return;

		auto best_cost{ 1'000'000 };
		auto best_node = current;
		explored.push_back(current);
		for (auto n : get_neighbors(current))
		{
			if (std::find(explored.begin(), explored.end(), n) != explored.end())
			{
				auto cost = evaluate(n);
				if (cost < best_cost) // Explore best node of neighbors
				{
					best_cost = cost;
					best_node = n;
				}
			}
		}

		if (best_node != current)
		{
			frontier.push(best_node);
			solution.add_to_solution(best_node);
		}
		else return; // Unable to find a better path, failure
	}
}

template <typename NodeType>
int DijkstraPathfinder<NodeType>::evaluate(const NodeType& node) const
{
	return 0;
}


#endif