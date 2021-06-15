#include "Pathfinding2D.h"
#include <functional>

PathfindingLayer::PathfindingLayer()
	: m_camera_controller{ 1.7778f }, m_maze{50, 50}, m_solution{ m_maze }
{
	//DijkstraPathfinder<Node> pathfinder{ m_maze.GetNeighborsVisitor };
}

void PathfindingLayer::OnAttach()
{

}

void PathfindingLayer::OnDetach()
{

}

void PathfindingLayer::OnUpdate(Kablunk::Timestep ts)
{
	m_camera_controller.OnUpdate(ts);
	Kablunk::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
	Kablunk::RenderCommand::Clear();

	Kablunk::Renderer2D::ResetStats();
	Kablunk::Renderer2D::BeginScene(m_camera_controller.GetCamera());

	Kablunk::Renderer2D::EndScene();
	m_maze.Render(m_camera_controller);
}

void PathfindingLayer::OnImGuiRender(Kablunk::Timestep ts)
{

}

void PathfindingLayer::OnEvent(Kablunk::Event& e)
{
	m_camera_controller.OnEvent(e);
}
