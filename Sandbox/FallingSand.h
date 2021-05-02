#pragma once
#include <Kablunk.h>

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
	enum TILE_BIT_DATA
	{
		Air = 0,
		Sand = 1,
		Water = 2
	};
private:
	void InitTileData(uint32_t width, uint32_t height);
	void UpdateAllTiles();
	uint32_t CoordToIndex(const glm::ivec2& pos);
	uint32_t CoordToIndex(uint32_t x, uint32_t y);

	bool IsInside(const glm::ivec2& pos);
	bool IsInside(uint32_t x, uint32_t y);

	bool Empty(const glm::ivec2& pos);
	bool Empty(uint32_t x, uint32_t y);

	bool MouseInsideSimulation(const glm::ivec2& pos);
	bool MouseInsideSimulation(uint32_t x, uint32_t y);

	void SetTile(const glm::ivec2& pos, TILE_BIT_DATA data);
	void SetTile(uint32_t x, uint32_t y, TILE_BIT_DATA data);
	
	bool UpdateTile(uint32_t x, uint32_t y, TILE_BIT_DATA bitData);
	glm::vec4 TileDataToColor(TILE_BIT_DATA bitData) const;
private:
	uint32_t m_ScreenWidth{ 1280 };
	uint32_t m_ScreenHeight{ 720 };

	uint32_t m_SimulationWidth{ 700 };
	uint32_t m_SimulationHeight{ 700 };

	TILE_BIT_DATA* m_TileData;
	int32_t m_TileRows{ 0 };
	int32_t m_TileCols{ 0 };

	float m_TileWidth;
	float m_TileHeight;

	Kablunk::OrthographicCameraController m_CameraController;

	float m_ImguiUpdateCounter = 0.0f;
	float m_ImguiUpdateCounterMax = .1f;
	float m_ImguiDeltaTime = 10.0f;
	float m_ImguiFPS = 10.0f;

	const float m_TicksPerSecond{ 1.0f / 20.0f };
	float m_TickCounter = 0.0f;
};

