#include <Kablunk.h>
#include "TileMap.h"

TileMap::TileMap()
	: m_TileData{ nullptr }, m_TileRows{ 0 }, m_TileCols{ 0 }, m_TileWidth{0.0f}, m_TileHeight{0.0f}
{
	
}

TileMap::TileMap(int rows, int cols)
	: m_TileData{ nullptr }, m_TileRows{rows}, m_TileCols{ cols }
{
	m_TileWidth = m_SimulationWidth / (float)rows;
	m_TileHeight = m_SimulationHeight / (float)cols;
	InitTileData();
}

TileMap::~TileMap()
{
	delete[] m_TileData;
}

void TileMap::InitTileData()
{
	m_TileData = new TILE_BIT_DATA[(uint64_t)(m_TileRows * m_TileCols)];
	for (uint32_t y = 0; y < m_TileCols; ++y)
	{
		for (uint32_t x = 0; x < m_TileRows; ++x)
		{
			if (y == 0)
				m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Sand;
			else
				m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Air;
		}
	}

	SetTile(99, 99, TILE_BIT_DATA::Water);
}

void TileMap::UpdateAllTiles()
{
	for (uint32_t y = 0; y < m_TileCols; ++y)
	{
		for (uint32_t x = 0; x < m_TileRows; ++x)
		{
			TILE_BIT_DATA tile = m_TileData[CoordToIndex(x, y)];
			if (tile != TILE_BIT_DATA::Air)
			{
				UpdateTile(x, y, tile);
			}
		}
	}
}

bool TileMap::Empty(uint32_t x, uint32_t y)
{
	KB_ASSERT(m_TileData != nullptr, "TileData not initialized!");
	if (!IsInside(x, y)) return false;
	return m_TileData[CoordToIndex(x, y)] == TILE_BIT_DATA::Air;
}
void TileMap::SetTile(uint32_t x, uint32_t y, TILE_BIT_DATA data)
{
	KB_ASSERT(IsInside(x, y), "Tile not inside map!");
	m_TileData[CoordToIndex(x, y)] = data;
}

bool TileMap::TrySetTile(uint32_t x, uint32_t y, TILE_BIT_DATA data)
{
	if (Empty(x, y))
	{
		m_TileData[CoordToIndex(x, y)] = data;
		return true;
	}
	else
		return false;
}

bool TileMap::UpdateTile(uint32_t x, uint32_t y, TILE_BIT_DATA bitData)
{
	switch (bitData)
	{
	case TILE_BIT_DATA::Air:    
		return false;
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

		// choose randomly between left and right
		int xDir = 1;
		if (rand() % 100 < 50)
			xDir = -1;

		// Try one direction
		move = { x + xDir, y - 1 };
		if (Empty(move))
		{
			m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
			m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Air;
			return true;
		}

		// Then try other
		move = { x + xDir * -1, y - 1 };
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

const glm::vec4 TileMap::TileDataToColor(TILE_BIT_DATA bitData)
{
	switch (bitData)
	{
	case TILE_BIT_DATA::Air:     return { 0.0f, 0.0f, 0.0f, 1.0f };
	case TILE_BIT_DATA::Sand:    return { 0.76f, 0.698f, 0.502f, 1.0f };
	case TILE_BIT_DATA::Water:   return { 0.678f, 0.847f, 0.902f, 1.0f };
	default:					 KB_ASSERT(false, "TILE NOT IMPLEMENTED");  return { 0.0f, 0.0f, 0.0f, 1.0f };
	}
}

