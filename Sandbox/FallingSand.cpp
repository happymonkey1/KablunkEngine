#include "FallingSand.h"


FallingSand::FallingSand()
	: Layer("FallingSand"), m_TileData{ nullptr }, m_CameraController{ 1.7778f }, m_TileWidth{0.0f}, m_TileHeight{0.0f}
{

}


void FallingSand::OnAttach()
{
	InitTileData(128, 72);
}

void FallingSand::OnDetach()
{
	delete[] m_TileData;
}

void FallingSand::OnUpdate(Kablunk::Timestep ts)
{
	KB_ASSERT(m_TileData != nullptr, "TileData not initialized!")
	// ==========
	//   Update
	// ==========

	m_CameraController.OnUpdate(ts);

	float rows{ (float)m_TileRows }, cols{ (float)m_TileCols };
	if (Kablunk::Input::IsMouseButtonPressed(KB_MOUSE_BUTTON_1))
	{
		float x = Kablunk::Input::GetMouseX(), y = Kablunk::Input::GetMouseY();
		
		if (MouseInsideSimulation(x, y))
		{
			glm::vec2 position = {
			((int)(float)(x / m_TileWidth) - ((float)m_SimulationWidth / 2.0f)),
			((int)((float)(m_ScreenHeight - y) / m_TileHeight))
			};

			if (Empty(position))
			{
				SetTile(position, TILE_BIT_DATA::Sand);
			}
		}
	}

	if (m_TickCounter >= m_TicksPerSecond)
	{
		UpdateAllTiles();
		m_TickCounter -= m_TicksPerSecond;
	}
	else
		m_TickCounter += ts.GetMiliseconds() / 1000.0f;

	// ==========
	//   Render
	// ==========

	Kablunk::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
	Kablunk::RenderCommand::Clear();

	Kablunk::Renderer2D::BeginScene(m_CameraController.GetCamera());

	glm::vec2 size{m_TileWidth / m_ScreenWidth, m_TileWidth / m_ScreenWidth };
	
	float simStartX = (float)(m_ScreenWidth - m_SimulationWidth) / 2.0f;
	float simStartY = (float)(m_ScreenHeight - m_SimulationHeight) / 2.0f;
	for (int y = 0; y < m_TileCols; ++y)
	{
		for (int x = 0; x < m_TileRows; ++x)
		{
			TILE_BIT_DATA tile = m_TileData[CoordToIndex(x, y)];
			if (tile != TILE_BIT_DATA::Air)
			{
				glm::vec2 position = {
					simStartX + (float)x * m_TileWidth,
					simStartY + (float)y * m_TileHeight
				};
				Kablunk::Renderer2D::DrawQuad(WorldToScreenPos(position), size, TileDataToColor(tile));
			}
		}
	}

	//Kablunk::Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 1.0f,1.0f }, TileDataToColor(TILE_BIT_DATA::Sand));

	Kablunk::Renderer2D::EndScene();
}

void FallingSand::OnImGuiRender(Kablunk::Timestep ts)
{
	if (m_ImguiUpdateCounter >= m_ImguiUpdateCounterMax)
	{
		float miliseconds = ts.GetMiliseconds();
		m_ImguiDeltaTime = miliseconds;
		m_ImguiFPS = 1000.0f / miliseconds;
		m_ImguiUpdateCounter -= m_ImguiUpdateCounterMax;
	}
	else
		m_ImguiUpdateCounter += ts.GetMiliseconds() / 1000.0f;

	ImGui::Begin("Debug Information");

	ImGui::Text("Frame time: %.*f", 4, m_ImguiDeltaTime);
	ImGui::Text("FPS: %.*f", 4, m_ImguiFPS);

	ImGui::End();
}

void FallingSand::OnEvent(Kablunk::Event& e)
{
	m_CameraController.OnEvent(e);

	Kablunk::EventDispatcher dispatcher{ e };
	dispatcher.Dispatch<Kablunk::WindowResizeEvent>(KABLUNK_BIND_EVENT_FN(FallingSand::OnWindowResize));
}

bool FallingSand::OnWindowResize(Kablunk::WindowResizeEvent& e)
{
	if (e.GetWidth() == 0 || e.GetHeight() == 0)
		return false;

	m_ScreenWidth = e.GetWidth();
	m_ScreenHeight = e.GetHeight();

	m_TileWidth = m_SimulationWidth / m_TileRows;
	m_TileHeight = m_SimulationHeight / m_TileCols;

	KB_CLIENT_INFO("Window resize called");
	return false;
}

void FallingSand::InitTileData(uint32_t width, uint32_t height)
{
	m_TileCols = height;
	m_TileRows = width;

	m_TileWidth = m_SimulationWidth / m_TileRows;
	m_TileHeight = m_SimulationHeight / m_TileCols;

	m_TileData = new TILE_BIT_DATA[(uint64_t)(width * height)];
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			if (y == 0)
				m_TileData[CoordToIndex(x,y)] = TILE_BIT_DATA::Sand;
			else
				m_TileData[CoordToIndex(x,y)] = TILE_BIT_DATA::Air;
		}
	}

	m_TileData[CoordToIndex(65, 32)] = TILE_BIT_DATA::Sand;
	m_TileData[CoordToIndex(100, 0)] = TILE_BIT_DATA::Sand;
	//m_TileData[CoordToIndex(0, 0)] = TILE_BIT_DATA::Sand;
	m_TileData[CoordToIndex(24, 55)] = TILE_BIT_DATA::Sand;
	m_TileData[CoordToIndex(112, 43)] = TILE_BIT_DATA::Sand;
	m_TileData[CoordToIndex(127, 71)] = TILE_BIT_DATA::Water;
}

void FallingSand::UpdateAllTiles()
{
	for (int y = 0; y < m_TileCols; ++y)
	{
		for (int x = 0; x < m_TileRows; ++x)
		{
			TILE_BIT_DATA tile = m_TileData[CoordToIndex(x, y)];
			if (tile != TILE_BIT_DATA::Air)
			{
				UpdateTile(x, y, tile);
			}
		}
	}
}

uint32_t FallingSand::CoordToIndex(const glm::vec2& pos) { return pos.y * m_TileRows + pos.x; }
uint32_t FallingSand::CoordToIndex(uint32_t x, uint32_t y) { return y * m_TileRows + x; }

bool FallingSand::IsInside(const glm::vec2& pos)
{
	return IsInside(pos.x, pos.y);
}

bool FallingSand::IsInside(uint32_t x, uint32_t y)
{
	return (x < m_TileRows&& x >= 0 && y < m_TileCols&& y >= 0);
}

bool FallingSand::Empty(const glm::vec2& pos)
{
	return Empty(pos.x, pos.y);
}

bool FallingSand::Empty(uint32_t x, uint32_t y)
{
	KB_ASSERT(m_TileData != nullptr, "TileData not initialized!");
	if (!IsInside(x, y)) return false;
	return m_TileData[CoordToIndex(x, y)] == TILE_BIT_DATA::Air;
}

bool FallingSand::MouseInsideSimulation(const glm::vec2& pos)
{
	return MouseInsideSimulation(pos.x, pos.y);
}

bool FallingSand::MouseInsideSimulation(uint32_t x, uint32_t y)
{
	uint32_t simLeftBound = ((float)m_ScreenWidth / 2.0f) - (m_TileRows / 2.0f * m_TileWidth);
	uint32_t simRightBound = ((float)m_ScreenWidth / 2.0f) + (m_TileRows / 2.0f * m_TileWidth);
	uint32_t simTopBound = ((float)m_ScreenHeight / 2.0f) + (m_TileCols / 2.0f * m_TileHeight); 
	uint32_t simBottomBound = ((float)m_ScreenHeight / 2.0f) - (m_TileCols / 2.0f * m_TileHeight);
	return (x > simLeftBound && x < simRightBound && y > simBottomBound && y < simTopBound);
}

void FallingSand::SetTile(const glm::vec2& pos, TILE_BIT_DATA data)
{
	SetTile(pos.x, pos.y, data);
}

void FallingSand::SetTile(uint32_t x, uint32_t y, TILE_BIT_DATA data)
{
	KB_ASSERT(IsInside(x, y), "Tile not inside map!");
	m_TileData[CoordToIndex(x, y)] = data;
}

glm::vec2 FallingSand::WorldToScreenPos(const glm::vec2& pos)
{
	return {
		(pos.x - (float)m_ScreenWidth / 2.0f) / ((float)m_ScreenWidth / 2.0f),
		(pos.y - (float)m_ScreenHeight / 2.0f) / ((float)m_ScreenHeight / 2.0f)
	};
}

bool FallingSand::UpdateTile(uint32_t x, uint32_t y, TILE_BIT_DATA bitData)
{
	switch (bitData)
	{
	case TILE_BIT_DATA::Air:
	{
		return false;
	}
	case TILE_BIT_DATA::Sand:
	{
		// Directly below
		glm::vec2 move{ x, y - 1 };
		if (Empty(move))
		{
			m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
			m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Air;
			return true;
		}
		return false;
	}
	case TILE_BIT_DATA::Water:   
	{
		return false;
	}
	default:
	{
		KB_ASSERT(false, "TILE NOT IMPLEMENTED"); 
		return false;
	}
	}
}
glm::vec4 FallingSand::TileDataToColor(TILE_BIT_DATA bitData) const
{
	switch (bitData)
	{
	case TILE_BIT_DATA::Air:     return { 0.0f, 0.0f, 0.0f, 1.0f };
	case TILE_BIT_DATA::Sand:    return { 0.76f, 0.698f, 0.502f, 1.0f };
	case TILE_BIT_DATA::Water:   return { 0.678f, 0.847f, 0.902f, 1.0f };
	default:					 KB_ASSERT(false, "TILE NOT IMPLEMENTED");  return { 0.0f, 0.0f, 0.0f, 1.0f };
	}
}
