#ifndef PATHFINDING_2D_H
#define PATHFINDING_2D_H

#include <Kablunk.h>
#include "Maze.h"
#include "Pathfinding.h"

class PathfindingLayer : public Kablunk::Layer
{
public:
	PathfindingLayer();
	virtual ~PathfindingLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Kablunk::Timestep ts) override;
	virtual void OnImGuiRender(Kablunk::Timestep ts) override;
	virtual void OnEvent(Kablunk::Event& e);

private:
	Kablunk::OrthographicCameraController m_camera_controller;
	Maze m_maze;
	Solution m_solution;
};

#endif