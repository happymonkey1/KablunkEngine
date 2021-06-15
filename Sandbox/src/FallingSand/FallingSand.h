#pragma once
#include <Kablunk.h>
#include "FallingSand/TileMap.h"
#include "FallingSand/Entities.h"

class FallingSand : public Kablunk::Layer
{
public:
	FallingSand();
	virtual ~FallingSand() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Kablunk::Timestep ts) override;
	virtual void OnImGuiRender(Kablunk::Timestep ts) override;
	virtual void OnEvent(Kablunk::Event & e);

	bool OnWindowResize(Kablunk::WindowResizeEvent& e);
public:
	
private:
	

	bool MouseInsideSimulation(const glm::ivec2& pos);
	bool MouseInsideSimulation(uint32_t x, uint32_t y);

	
private:
	uint32_t m_ScreenWidth{ 1280 };
	uint32_t m_ScreenHeight{ 720 };

	TileMap m_TileMap;

	Kablunk::OrthographicCameraController m_CameraController;

	TileType m_CurrentTileTypeSelected = TileType::Sand;
	int32_t m_BrushRadius{ 1 };

	Frog m_Frog1{ glm::vec3{0.0f}, glm::vec2{0.0f}, glm::vec2{0.0f} };

	const float m_TicksPerSecond{ 1.0f / 20.0f };
	float m_TickCounter = 0.0f;
};

