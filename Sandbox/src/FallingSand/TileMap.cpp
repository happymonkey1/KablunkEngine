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
	std::vector <std::pair<glm::vec2, TILE_BIT_DATA>> gasTiles;
	for (int32_t y = 0; y < m_TileCols; ++y)//for (int32_t y = m_TileCols - 1; y >= 0; --y)
	{
		for (int32_t x = 0; x < m_TileRows; ++x)//for (int32_t x = m_TileRows - 1; x >= 0; --x)
		{
			TILE_BIT_DATA tile = m_TileData[CoordToIndex(x, y)];
			if (tile != TILE_BIT_DATA::Air && tile != Smoke && tile != Steam)
			{
				UpdateTile(x, y, tile);
			}
			else if (tile == Smoke || tile == Steam)
			{
				gasTiles.push_back(std::make_pair(glm::vec2{ x, y }, tile));
			}
		}
	}

	// Reverse order for gases
	size_t s = gasTiles.size();
	for (int i = s - 1; i >= 0; --i)
	{
		UpdateTile(gasTiles[i].first.x, gasTiles[i].first.y, gasTiles[i].second);
	}
}

void TileMap::Reset()
{
	memset(m_TileData, Air, sizeof(TILE_BIT_DATA) * m_TileRows * m_TileCols);
}

bool TileMap::Empty(uint32_t x, uint32_t y)
{
	KB_ASSERT(m_TileData != nullptr, "TileData not initialized!");
	if (!IsInside(x, y)) return false;
	return m_TileData[CoordToIndex(x, y)] == TILE_BIT_DATA::Air;
}

TileMap::TILE_BIT_DATA TileMap::At(uint32_t x, uint32_t y)
{
	if (!IsInside(x, y)) KB_CORE_ASSERT(false, "Trying to get a tile that is not inside the map.");
	return m_TileData[CoordToIndex(x, y)];
}

void TileMap::SetTile(uint32_t x, uint32_t y, TILE_BIT_DATA data)
{
	KB_ASSERT(IsInside(x, y), "Tile not inside map!");
	m_TileData[CoordToIndex(x, y)] = data;
}

bool TileMap::TrySetTile(uint32_t x, uint32_t y, TILE_BIT_DATA data)
{
	if (IsInside(x, y))
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
	case TILE_BIT_DATA::Sand:  // TODO: Make sand randomly disperse in water
	{
		// Directly below
		glm::vec2 move{ x, y - 1 };
		if (IsInside(move))
		{
			TILE_BIT_DATA t = At(move);
			if (IsGas(t))
			{
				TILE_BIT_DATA old = m_TileData[CoordToIndex(move)];
				m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
				m_TileData[CoordToIndex(x, y)] = old;
				return true;
			}
			else if (IsLiquid(t))
			{
				while (true)  // FIND BETTER WAY TO DO THIS
				{
					int r = rand() % 100;
					int xDir = 1;
					if (r <= 33)
						xDir = -1;
					else if (r > 33 && r <= 66)
						xDir = 0;
					
					move = { x + xDir, y - 1 };
					if (IsInside(move))
					{
						TILE_BIT_DATA dispersedTile = m_TileData[CoordToIndex(move)];
						if (IsGas(dispersedTile) || IsLiquid(dispersedTile))
						{
							TILE_BIT_DATA old = m_TileData[CoordToIndex(move)];
							m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
							m_TileData[CoordToIndex(x, y)] = old;
							return true;
						}
					}
				}
			}
		}

		// choose randomly between left and right
		int xDir = 1;
		if (rand() % 100 < 50)
			xDir = -1;

		// Try one direction
		move = { x + xDir, y - 1 };
		if (IsInside(move))
		{
			TILE_BIT_DATA t = At(move);
			if (IsGas(t) || IsLiquid(t))
			{
				TILE_BIT_DATA old = m_TileData[CoordToIndex(move)];
				m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
				m_TileData[CoordToIndex(x, y)] = old;
				return true;
			}
		}

		// Then try other
		move = { x + xDir * -1, y - 1 };
		if (IsInside(move))
		{
			TILE_BIT_DATA t = At(move);
			if (IsGas(t) || IsLiquid(t))
			{
				TILE_BIT_DATA old = m_TileData[CoordToIndex(move)];
				m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
				m_TileData[CoordToIndex(x, y)] = old;
				return true;
			}
		}

		return false;
	}
	case TILE_BIT_DATA::Water:  // TODO: Make water more realistic
	{
		// Directly below
		glm::vec2 move{ x, y - 1 };
		if (IsInside(move))
		{
			TILE_BIT_DATA t = At(x, y - 1);
			if (Empty(move) || IsGas(t))
			{
				TILE_BIT_DATA old = m_TileData[CoordToIndex(move)];
				m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
				m_TileData[CoordToIndex(x, y)] = old;
				return true;
			}
			else if (t == TILE_BIT_DATA::Lava) // WATER + LAVA = STEAM
			{
				m_TileData[CoordToIndex(move)] = TILE_BIT_DATA::Steam;
				m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Stone;
				return true;
			}
		}
		// choose randomly between down left and down right
		int xDir = 1;
		if (rand() % 100 < 50)
			xDir = -1;

		// Try one direction
		move = { x + xDir, y - 1 };
		if (IsInside(move))
		{
			TILE_BIT_DATA t = At(move);
			if (Empty(move) || IsGas(t))
			{
				TILE_BIT_DATA old = m_TileData[CoordToIndex(move)];
				m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
				m_TileData[CoordToIndex(x, y)] = old;
				return true;
			}
			else if (t == TILE_BIT_DATA::Lava) // WATER + LAVA = STEAM
			{
				m_TileData[CoordToIndex(move)] = TILE_BIT_DATA::Steam;
				m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Stone;
				return true;
			}
		}
		// Then try other
		move = { x + xDir * -1, y - 1 };
		if (IsInside(move))
		{
			TILE_BIT_DATA t = At(move);
			if (Empty(move) || IsGas(t))
			{
				TILE_BIT_DATA old = m_TileData[CoordToIndex(move)];
				m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
				m_TileData[CoordToIndex(x, y)] = old;
				return true;
			}
			else if (t == TILE_BIT_DATA::Lava) // WATER + LAVA = STEAM
			{
				m_TileData[CoordToIndex(move)] = TILE_BIT_DATA::Steam;
				m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Stone;
			}
		}

		bool foundAirLeft = false;
		int32_t xPosLeft = x;
		for (int nextRowX = x - 1; nextRowX >= 0; --nextRowX)
		{
			move = { nextRowX, y - 1 };
			if (!IsInside(move))
				break;
			TILE_BIT_DATA t = At(move);
			if (IsGas(t))
			{
				foundAirLeft = true;
				xPosLeft = nextRowX;
				break;
			}
		}

		bool foundAirRight = false;
		int32_t xPosRight = x;
		for (int nextRowX = x + 1; nextRowX < m_TileRows; ++nextRowX)
		{
			move = { nextRowX, y - 1 };
			if (!IsInside(move))
				break;
			TILE_BIT_DATA t = At(move);
			if (IsGas(t))
			{
				foundAirRight = true;
				xPosRight = nextRowX;
				break;
			}
		}
		if (!foundAirLeft && !foundAirRight)
			return false;
		else if (foundAirRight && foundAirLeft)
		{
			// now try randomly choosing between horizontal left and horizontal right
			uint32_t distToLeft = std::abs(static_cast<int32_t>(x) - xPosLeft);
			uint32_t distToRight = std::abs(static_cast<int32_t>(x) - xPosRight);

			int xDir = 1;
			if (distToRight < distToLeft)
				xDir = 1;
			else if (distToLeft < distToRight)
				xDir = -1;
			else
			{
				if (rand() % 100 < 50)
					xDir = -1;
			}
			// Try one direction
			move = { x + xDir, y };
			if (IsInside(move))
			{
				TILE_BIT_DATA t = At(move);
				if (Empty(move) || IsGas(t))
				{
					TILE_BIT_DATA old = m_TileData[CoordToIndex(move)];
					m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
					m_TileData[CoordToIndex(x, y)] = old;
					return true;
				}
				else if (t == TILE_BIT_DATA::Lava) // WATER + LAVA = STEAM
				{
					m_TileData[CoordToIndex(move)] = TILE_BIT_DATA::Steam;
					m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Stone;
					return true;
				}
			}
		}
		else if (foundAirLeft)
		{
			move = { x - 1, y };
			if (IsInside(move))
			{
				TILE_BIT_DATA t = At(move);
				if (Empty(move) || IsGas(t))
				{
					TILE_BIT_DATA old = m_TileData[CoordToIndex(move)];
					m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
					m_TileData[CoordToIndex(x, y)] = old;
					return true;
				}
				else if (t == TILE_BIT_DATA::Lava) // WATER + LAVA = STEAM
				{
					m_TileData[CoordToIndex(move)] = TILE_BIT_DATA::Steam;
					m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Stone;
					return true;
				}
			}
		}
		else if (foundAirRight)
		{
			move = { x + 1, y };
			if (IsInside(move))
			{
				TILE_BIT_DATA t = At(move);
				if (Empty(move) || IsGas(t))
				{
					TILE_BIT_DATA old = m_TileData[CoordToIndex(move)];
					m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
					m_TileData[CoordToIndex(x, y)] = old;
					return true;
				}
				else if (t == TILE_BIT_DATA::Lava) // WATER + LAVA = STEAM
				{
					m_TileData[CoordToIndex(move)] = TILE_BIT_DATA::Steam;
					m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Stone;
					return true;
				}
			}
		}

		return false;
	}
	case Lava:
	{
		bool startedFire = false;
		for (int32_t moveY = -1; moveY <= 1; ++moveY)
		{
			for (int32_t moveX = -1; moveX <= 1; ++moveX)
			{
				glm::ivec2 move{ x + moveX, y + moveY };
				if (IsInside(move))
				{
					TILE_BIT_DATA t = At(move);
					if (IsFlammable(t))
					{
						// If tile around is flammable, 
						// 10 % chance it catches fire
						if (rand() % 1000 < 5)
						{
							SetTile(move, Fire);
							startedFire = true;
							break;
						}
					}
				}

			}

			if (startedFire)
				break;
		}


		// Directly below
		glm::vec2 move{ x, y - 1 };
		if (IsInside(move))
		{
			TILE_BIT_DATA t = At(x, y - 1);
			if (Empty(move) || IsGas(t))
			{
				TILE_BIT_DATA old = m_TileData[CoordToIndex(move)];
				m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
				m_TileData[CoordToIndex(x, y)] = old;
				return true;
			}
			else if (t == TILE_BIT_DATA::Water) // WATER + LAVA = STEAM
			{
				m_TileData[CoordToIndex(move)] = TILE_BIT_DATA::Steam;
				m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Stone;
				return true;
			}
		}
		// choose randomly between down left and down right
		int xDir = 1;
		if (rand() % 100 < 50)
			xDir = -1;

		// Try one direction
		move = { x + xDir, y - 1 };
		if (IsInside(move))
		{
			TILE_BIT_DATA t = At(move);
			if (Empty(move) || IsGas(t))
			{
				TILE_BIT_DATA old = m_TileData[CoordToIndex(move)];
				m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
				m_TileData[CoordToIndex(x, y)] = old;
				return true;
			}
			else if (t == TILE_BIT_DATA::Water) // WATER + LAVA = STEAM
			{
				m_TileData[CoordToIndex(move)] = TILE_BIT_DATA::Steam;
				m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Stone;
				return true;
			}
		}
		// Then try other
		move = { x + xDir * -1, y - 1 };
		if (IsInside(move))
		{
			TILE_BIT_DATA t = At(move);
			if (Empty(move) || IsGas(t))
			{
				TILE_BIT_DATA old = m_TileData[CoordToIndex(move)];
				m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
				m_TileData[CoordToIndex(x, y)] = old;
				return true;
			}
			else if (t == TILE_BIT_DATA::Water) // WATER + LAVA = STEAM
			{
				m_TileData[CoordToIndex(move)] = TILE_BIT_DATA::Steam;
				m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Stone;
			}
		}

		bool foundAirLeft = false;
		int32_t xPosLeft = x;
		for (int nextRowX = x - 1; nextRowX >= 0; --nextRowX)
		{
			move = { nextRowX, y - 1 };
			if (!IsInside(move))
				break;
			TILE_BIT_DATA t = At(move);
			if (IsGas(t))
			{
				foundAirLeft = true;
				xPosLeft = nextRowX;
				break;
			}
		}

		bool foundAirRight = false;
		int32_t xPosRight = x;
		for (int nextRowX = x + 1; nextRowX < m_TileRows; ++nextRowX)
		{
			move = { nextRowX, y - 1 };
			if (!IsInside(move))
				break;
			TILE_BIT_DATA t = At(move);
			if (IsGas(t))
			{
				foundAirRight = true;
				xPosRight = nextRowX;
				break;
			}
		}
		if (!foundAirLeft && !foundAirRight)
			return false;
		else if (foundAirRight && foundAirLeft)
		{
			// now try randomly choosing between horizontal left and horizontal right
			uint32_t distToLeft = std::abs(static_cast<int32_t>(x) - xPosLeft);
			uint32_t distToRight = std::abs(static_cast<int32_t>(x) - xPosRight);

			int xDir = 1;
			if (distToRight < distToLeft)
				xDir = 1;
			else if (distToLeft < distToRight)
				xDir = -1;
			else
			{
				if (rand() % 100 < 50)
					xDir = -1;
			}
			// Try one direction
			move = { x + xDir, y };
			if (IsInside(move))
			{
				TILE_BIT_DATA t = At(move);
				if (Empty(move) || IsGas(t))
				{
					TILE_BIT_DATA old = m_TileData[CoordToIndex(move)];
					m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
					m_TileData[CoordToIndex(x, y)] = old;
					return true;
				}
				else if (t == TILE_BIT_DATA::Water) // WATER + LAVA = STEAM
				{
					m_TileData[CoordToIndex(move)] = TILE_BIT_DATA::Steam;
					m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Stone;
					return true;
				}
			}
		}
		else if (foundAirLeft)
		{
			move = { x - 1, y };
			if (IsInside(move))
			{
				TILE_BIT_DATA t = At(move);
				if (Empty(move) || IsGas(t))
				{
					TILE_BIT_DATA old = m_TileData[CoordToIndex(move)];
					m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
					m_TileData[CoordToIndex(x, y)] = old;
					return true;
				}
				else if (t == TILE_BIT_DATA::Water) // WATER + LAVA = STEAM
				{
					m_TileData[CoordToIndex(move)] = TILE_BIT_DATA::Steam;
					m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Stone;
					return true;
				}
			}
		}
		else if (foundAirRight)
		{
			move = { x + 1, y };
			if (IsInside(move))
			{
				TILE_BIT_DATA t = At(move);
				if (Empty(move) || IsGas(t))
				{
					TILE_BIT_DATA old = m_TileData[CoordToIndex(move)];
					m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
					m_TileData[CoordToIndex(x, y)] = old;
					return true;
				}
				else if (t == TILE_BIT_DATA::Water) // WATER + LAVA = STEAM
				{
					m_TileData[CoordToIndex(move)] = TILE_BIT_DATA::Steam;
					m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Stone;
					return true;
				}
			}
		}

		return false;
	}
	case Steam:
	{
		// choose randomly between up, up left, and up right
		int xDir = 0;
		int r = rand() % 100;
		if (r < 33)
			xDir = -1;
		else if (r >= 33 && r < 66)
			xDir = 1;

		// Try one direction
		glm::vec2 move = { x + xDir, y + 1 };
		if (Empty(move))
		{
			m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
			m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Air;
			return true;
		}

		int newXDir;
		if (xDir == 0)
		{
			newXDir = 1;
			if (rand() % 100 < 50)
				newXDir = -1;
		}
		else
		{
			newXDir = xDir * -1;
			if (rand() % 100 < 50)
				newXDir = 0;
		}

		// Then try other
		move = { x + newXDir, y + 1 };
		if (Empty(move))
		{
			m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
			m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Air;
			return true;
		}

		//Finally try last direction
		int finalXDir = (newXDir + xDir) * -1;
		move = { x + finalXDir, y + 1 };
		if (Empty(move))
		{
			m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
			m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Air;
			return true;
		}

		bool foundAirLeft = false;
		int32_t xPosLeft = x;
		for (int nextRowX = x - 1; nextRowX >= 0; --nextRowX)
		{
			move = { nextRowX, y + 1 };
			if (!IsInside(move))
				break;
			TILE_BIT_DATA t = At(move);
			if (t == TILE_BIT_DATA::Air)
			{
				foundAirLeft = true;
				xPosLeft = nextRowX;
				break;
			}
		}

		bool foundAirRight = false;
		int32_t xPosRight = x;
		for (int nextRowX = x + 1; nextRowX < m_TileRows; ++nextRowX)
		{
			move = { nextRowX, y + 1 };
			if (!IsInside(move))
				break;
			TILE_BIT_DATA t = At(move);
			if (t == TILE_BIT_DATA::Air)
			{
				foundAirRight = true;
				xPosRight = nextRowX;
				break;
			}
		}
		if (!foundAirLeft && !foundAirRight)
			return false;
		else if (foundAirRight && foundAirLeft)
		{
			// now try randomly choosing between horizontal left and horizontal right
			uint32_t distToLeft = std::abs(static_cast<int32_t>(x) - xPosLeft);
			uint32_t distToRight = std::abs(static_cast<int32_t>(x) - xPosRight);

			int xDir = 1;
			if (distToRight < distToLeft)
				xDir = 1;
			else if (distToLeft < distToRight)
				xDir = -1;
			else
			{
				if (rand() % 100 < 50)
					xDir = -1;
			}
			// Try one direction
			move = { x + xDir, y };
			if (Empty(move))
			{
				m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
				m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Air;
				return true;
			}
		}
		else if (foundAirLeft)
		{
			move = { x - 1, y };
			if (Empty(move))
			{
				m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
				m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Air;
				return true;
			}
		}
		else if (foundAirRight)
		{
			move = { x + 1, y };
			if (Empty(move))
			{
				m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
				m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Air;
				return true;
			}
		}

		return false;
	}
	case Smoke:
	{
		// choose randomly between up, up left, and up right
		int xDir = 0;
		int r = rand() % 100;
		if (r < 33)
			xDir = -1;
		else if (r >= 33 && r < 66)
			xDir = 1;

		// Try one direction
		glm::vec2 move = { x + xDir, y + 1 };
		if (Empty(move))
		{
			m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
			m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Air;
			return true;
		}

		int newXDir;
		if (xDir == 0)
		{
			newXDir = 1;
			if (rand() % 100 < 50)
				newXDir = -1;
		}
		else
		{
			newXDir = xDir * -1;
			if (rand() % 100 < 50)
				newXDir = 0;
		}

		// Then try other
		move = { x + newXDir, y + 1 };
		if (Empty(move))
		{
			m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
			m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Air;
			return true;
		}

		//Finally try last direction
		int finalXDir = (newXDir + xDir) * -1;
		move = { x + finalXDir, y + 1 };
		if (Empty(move))
		{
			m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
			m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Air;
			return true;
		}

		bool foundAirLeft = false;
		int32_t xPosLeft = x;
		for (int nextRowX = x - 1; nextRowX >= 0; --nextRowX)
		{
			move = { nextRowX, y + 1 };
			if (!IsInside(move))
				break;
			TILE_BIT_DATA t = At(move);
			if (t == TILE_BIT_DATA::Air)
			{
				foundAirLeft = true;
				xPosLeft = nextRowX;
				break;
			}
		}

		bool foundAirRight = false;
		int32_t xPosRight = x;
		for (int nextRowX = x + 1; nextRowX < m_TileRows; ++nextRowX)
		{
			move = { nextRowX, y + 1 };
			if (!IsInside(move))
				break;
			TILE_BIT_DATA t = At(move);
			if (t == TILE_BIT_DATA::Air)
			{
				foundAirRight = true;
				xPosRight = nextRowX;
				break;
			}
		}
		if (!foundAirLeft && !foundAirRight)
			return false;
		else if (foundAirRight && foundAirLeft)
		{
			// now try randomly choosing between horizontal left and horizontal right
			uint32_t distToLeft = std::abs(static_cast<int32_t>(x) - xPosLeft);
			uint32_t distToRight = std::abs(static_cast<int32_t>(x) - xPosRight);

			int xDir = 1;
			if (distToRight < distToLeft)
				xDir = 1;
			else if (distToLeft < distToRight)
				xDir = -1;
			else
			{
				if (rand() % 100 < 50)
					xDir = -1;
			}
			// Try one direction
			move = { x + xDir, y };
			if (Empty(move))
			{
				m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
				m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Air;
				return true;
			}
		}
		else if (foundAirLeft)
		{
			move = { x - 1, y };
			if (Empty(move))
			{
				m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
				m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Air;
				return true;
			}
		}
		else if (foundAirRight)
		{
			move = { x + 1, y };
			if (Empty(move))
			{
				m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(x, y)];
				m_TileData[CoordToIndex(x, y)] = TILE_BIT_DATA::Air;
				return true;
			}
		}

		return false;
	}
	case Stone:
		return false;
	case Wood:
		return false;
	case Fire:
	{
		// 5 % chance fire dies out
		if (rand() % 100 < 2)
		{
			m_TileData[CoordToIndex(x, y)] = Smoke;
			return true;
		}

		// 3 % chance to produce smoke
		if (rand() % 100 < 3)
		{
			glm::ivec2 move{ x, y + 1 };
			if (IsInside(move))
			{
				if (Empty(move))
				{
					SetTile(x, y, Smoke);
				}
			}
		}

		// 35 % chance fire dies out
		bool startedFire = false;
		for (int32_t moveY = -1; moveY <= 1; ++moveY)
		{
			for (int32_t moveX = -1; moveX <= 1; ++moveX)
			{
				glm::ivec2 move{ x + moveX, y + moveY };
				if (IsInside(move))
				{
					TILE_BIT_DATA t = At(move);
					if (IsFlammable(t))
					{
						// If tile around is flammable, 
						// 10 % chance it catches fire
						if (rand() % 100 < 7)
						{
							SetTile(move, Fire);
							startedFire = true;
							break;
						}
					}
				}
				
			}

			if (startedFire)
				break;
		}

		


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
	case TILE_BIT_DATA::Sand:    return { 0.760f, 0.698f, 0.502f, 1.0f };
	case TILE_BIT_DATA::Water:   return { 0.678f, 0.847f, 0.902f, 1.0f };
	case TILE_BIT_DATA::Lava:    return { 0.812f, 0.063f, 0.125f, 1.0f };
	case TILE_BIT_DATA::Steam:   return { 0.533f, 0.549f, 0.553f, 1.0f }; 
	case TILE_BIT_DATA::Smoke:   return { 0.961f, 0.961f, 0.961f, 0.1f }; 
	case TILE_BIT_DATA::Stone:   return { 0.533f, 0.549f, 0.553f, 0.1f };
	case TILE_BIT_DATA::Wood:    return { 0.572f, 0.526f, 0.271f, 1.0f };
	case TILE_BIT_DATA::Fire:    return { 0.886f, 0.345f, 0.133f, 1.0f };
	default:					 KB_ASSERT(false, "TILE NOT IMPLEMENTED");  return { 0.0f, 0.0f, 0.0f, 1.0f };
	}
}

