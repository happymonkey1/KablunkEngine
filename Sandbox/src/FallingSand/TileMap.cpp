#include <Kablunk.h>
#include "TileMap.h"

TileMap::TileMap()
	: m_TileData{ nullptr }, m_TileRows{ 0 }, m_TileCols{ 0 }, m_TileWidth{0.0f}, m_TileHeight{0.0f}
{
	
}

TileMap::TileMap(int rows, int cols)
	: m_TileData{ nullptr }, m_TileRows{rows}, m_TileCols{ cols }
{
	m_TileWidth = m_SimulationWidth / static_cast<float>(rows);
	m_TileHeight = m_SimulationHeight / static_cast<float>(cols);
	InitTileData();
}

TileMap::~TileMap()
{
	delete[] m_TileData;
}

void TileMap::InitTileData()
{
	m_TileData = new Tile[m_TileRows * m_TileCols];
	for (uint32_t y = 0; y < m_TileCols; ++y)
	{
		for (uint32_t x = 0; x < m_TileRows; ++x)
		{
			int i = CoordToIndex(x, y);
			if (y == 0)
				m_TileData[i] = DefaultTiles[TileType::Sand];
			else
				m_TileData[i] = DefaultTiles[TileType::Air];

			m_TileData[i].SetPosition(x, y);
		}
	}

	SetTile(99, 99, TileType::Water);
}

void TileMap::UpdateAllTiles()
{
	std::vector <std::pair<glm::vec2, TileType>> gasTiles;
	for (int32_t y = 0; y < m_TileCols; ++y)//for (int32_t y = m_TileCols - 1; y >= 0; --y)
	{
		for (int32_t x = 0; x < m_TileRows; ++x)//for (int32_t x = m_TileRows - 1; x >= 0; --x)
		{
			Tile tile = At(x, y);
			if (tile.Type != TileType::Air && !tile.HasUpdated)
			{
				m_TileData[CoordToIndex(x, y)].HasUpdated = true;
				UpdateTile(x, y, tile.Type);
			}
		}
	}

	m_NeedToFlagTilesForUpdate = true;
}

void TileMap::Reset()
{
	memset(m_TileData, 0, sizeof(TileType) * m_TileRows * m_TileCols);
}

void TileMap::FlagTilesForUpdate()
{
	if (!m_NeedToFlagTilesForUpdate) return;

	for (int32_t y = 0; y < m_TileCols; ++y)
	{
		for (int32_t x = 0; x < m_TileRows; ++x)
		{
			if (GetTypeAt(x, y) != TileType::Air)
			{
				m_TileData[CoordToIndex(x, y)].HasUpdated = false;
			}
		}
	}

	m_NeedToFlagTilesForUpdate = false;
}

bool TileMap::Empty(uint32_t x, uint32_t y)
{
	KB_ASSERT(m_TileData != nullptr, "TileData not initialized!");
	if (!IsInside(x, y)) return false;
	return m_TileData[CoordToIndex(x, y)] == TileType::Air;
}

const TileMap::Tile& TileMap::At(uint32_t x, uint32_t y)
{
	if (!IsInside(x, y)) KB_CORE_ASSERT(false, "Trying to get a tile that is not inside the map.");
	return m_TileData[CoordToIndex(x, y)];
}

void TileMap::SetTile(uint32_t x, uint32_t y, TileType data)
{
	KB_ASSERT(IsInside(x, y), "Tile not inside map!");
	m_TileData[CoordToIndex(x, y)].Type = data;
}

bool TileMap::TrySetTile(uint32_t x, uint32_t y, TileType data)
{
	if (IsInside(x, y))
	{
		m_TileData[CoordToIndex(x, y)].Type = data;
		return true;
	}
	else
		return false;
}

bool TileMap::UpdateTile(uint32_t x, uint32_t y, TileType bitData)
{
	switch (bitData)
	{
	case TileType::Air:    
		return false;
	case TileType::Sand:  
	{
		if (MoveDown(x, y))
			return true;
		else if (MoveDownSide(x, y))
			return true;
		else
			return false;
	}
	case TileType::Water:  // TODO: Make water more realistic
	{
		if (MoveDown(x, y))
			return true;
		else if (MoveDownSide(x, y))
			return true;
		else if (MoveSide(x, y))
			return true;
		else
			return false;
	}
	case TileType::Lava:
	{
		
		SpreadRandom(x, y);

		if (MoveDown(x, y))
			return true;
		else if (MoveDownSide(x, y))
			return true;
		else if (MoveSide(x, y))
			return true;
		else
			return false;
	}
	case TileType::Steam:
	{
		if (MoveDown(x, y, true))
			return true;
		else if (MoveDownSide(x, y, true))
			return true;
		else if (MoveSide(x, y))
			return true;
		else
			return false;
	}
	case TileType::Smoke:
	{
		if (MoveDown(x, y, true))
			return true;
		else if (MoveDownSide(x, y, true))
			return true;
		else if (MoveSide(x, y))
			return true;
		else
			return false;
	}
	case TileType::Stone:   return false;
	case TileType::Wood:    return false;
	case TileType::Fire:
	{
		// 5 % chance fire dies out
		if (rand() % 100 < 2)
		{
			m_TileData[CoordToIndex(x, y)] = DefaultTiles[TileType::Smoke];
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
					SetTile(x, y, TileType::Smoke);
				}
			}
		}

		return SpreadRandom(x, y);
	}
	default:
	{
		KB_ASSERT(false, "TILE NOT IMPLEMENTED");
		return false;
	}
	}
}

void TileMap::SwapTiles(glm::vec2 start, glm::vec2 move)
{
	Tile old = At(move);
	m_TileData[CoordToIndex(move)] = m_TileData[CoordToIndex(start)];
	m_TileData[CoordToIndex(start)] = old;
	m_TileData[CoordToIndex(move)].SetPosition(move);
	m_TileData[CoordToIndex(start)].SetPosition(start);
}

bool TileMap::MoveDown(uint32_t x, uint32_t y, bool reversedGravity /*= false*/)
{
	Tile start = At(x, y);
	glm::vec2 move = start.Position;
	auto potential_move = move;
	for (int i = 1; i < m_TileSettings.Gravity+1; ++i)
	{
		potential_move.y -= !reversedGravity ? 1 : -1;
		if (IsInside(potential_move))
		{
			TileType t = GetTypeAt(potential_move);
			if (IsMixable(start.Type, t)) return MoveMix(potential_move.x, potential_move.y + i * (!reversedGravity ? 1 : -1), potential_move.x, potential_move.y);
			else if (!IsGas(start.Type) && IsGas(t) || (IsGas(start.Type) && IsLiquid(t)))
			{
				move = potential_move;
				continue;
			}
			else if (start.MoveProperties & MoveProps::Disperse)
			{
				int count = 0;
				while (true)  // FIND BETTER WAY TO DO THIS
				{
					if (count >= 4)
						break;
					int r = rand() % 100;
					int xDir = 1;
					if (r <= 33)	xDir = -1;
					else if (r > 33 && r <= 66) xDir = 0;

					glm::vec2 dispersedMove = { potential_move.x + xDir, potential_move.y };
					if (IsInside(dispersedMove))
					{
						TileType dispersedTile = GetTypeAt(dispersedMove);
						if (IsGas(dispersedTile) || IsLiquid(dispersedTile))
						{
							SwapTiles({ potential_move.x, potential_move.y }, dispersedMove);
							return true;
						}
					}
					count++;
				}
			}
			else
				break;
		}
		else
			break;
	}

	if (IsInside(move) && move != static_cast<glm::vec2>(start.Position))
	{
		SwapTiles(start.Position, move);
		return true;
	}
	else
		return false;
}

bool TileMap::MoveDownSide(uint32_t x, uint32_t y, bool reversedGravity /*= false*/)
{
	int xDir = 1;
	if (rand() % 100 < 50)
		xDir = -1;

	// Try one direction
	Tile start = At(x, y);
	glm::vec2 move;
	int slipFactor = std::max(1, static_cast<int>(std::round(static_cast<float>(m_TileSettings.Gravity) * (1 - m_TileSettings.Friction))));
	bool foundMove = false;
	for (int i = 1; i < slipFactor + 1; ++i)
	{
		move = { x + xDir * i, y - i * (!reversedGravity ? 1 : -1) };
		if (IsInside(move))
		{
			TileType t = GetTypeAt(move);
			if (IsMixable(GetTypeAt(x, y), t)) return MoveMix(x, y, move.x, move.y);
			else if (GetTypeAt(x, y) != t && (IsGas(t) || IsLiquid(t)) && !IsGas(GetTypeAt({ move.x, move.y - 1 * (!reversedGravity ? 1 : -1) })))
			{
				foundMove = true; continue;
			}
			else
			{
				move.x -= xDir;
				move.y += 1 * (!reversedGravity ? 1 : -1);  
				break;
			}
		}
		else
		{
			move.x -= xDir;
			move.y += 1 * (!reversedGravity ? 1 : -1);
			break;
		}
	}

	if (foundMove && IsInside(move))
	{
		SwapTiles(start.Position, move);
		return true;
	}



	foundMove = false;
	for (int i = 1; i < slipFactor + 1; ++i)
	{
		move = { x + xDir * -1 * i, y - i * (!reversedGravity ? 1 : -1) };
		if (IsInside(move))
		{
			TileType t = GetTypeAt(move);
			if (IsMixable(GetTypeAt(x, y), t)) return MoveMix(x, y, move.x, move.y);
			else if (GetTypeAt(x, y) != t && (IsGas(t) || IsLiquid(t)) && !IsGas(GetTypeAt({ move.x, move.y - 1 * (!reversedGravity ? 1 : -1) })))
			{
				foundMove = true; continue;
			}
			else
			{
				move.x -= xDir * -1;
				move.y += 1 * (!reversedGravity ? 1 : -1);
				break;
			}
		}
		else
		{
			move.x -= xDir * -1;
			move.y += 1 * (!reversedGravity ? 1 : -1);
			break;
		}
	}

	if (foundMove && IsInside(move))
	{
		SwapTiles(start.Position, move);
		return true;
	}
	else
		return false;

#ifdef OLD_PATH
	if (IsInside(move))
	{
		TileType t = GetTypeAt(move);
		if (IsMixable(GetTypeAt(x, y), t)) return MoveMix(x, y, move.x, move.y);
		else if (GetTypeAt(x, y) != t && (IsGas(t) || IsLiquid(t)))
		{
			SwapTiles({x, y}, move);
			return true;
		}
		
	}

	// Then try other
	move = { x + xDir * -1, y - (!reversedGravity ? 1 : -1) };
	if (IsInside(move))
	{
		TileType t = GetTypeAt(move);
		if (IsMixable(GetTypeAt(x, y), t)) return MoveMix(x, y, move.x, move.y);
		else if (GetTypeAt(x, y) != t && (IsGas(t) || IsLiquid(t)))
		{
			SwapTiles({ x, y }, move);
			return true;
		}
	}

	return false;
#endif
}

bool TileMap::MoveSide(uint32_t x, uint32_t y)
{
	glm::vec2 move;
	bool foundAirLeft = false;
	int32_t xPosLeft = x;
	for (int nextRowX = x - 1; nextRowX >= 0; --nextRowX)
	{
		move = { nextRowX, y - 1 };
		if (!IsInside(move))
			break;
		TileType t = GetTypeAt(move);
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
		TileType t = GetTypeAt(move);
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
			TileType t = GetTypeAt(move);
			if (IsMixable(GetTypeAt(x, y), t)) return MoveMix(x, y, move.x, move.y);
			else if (IsGas(t))
			{
				SwapTiles({ x, y }, move);
				return true;
			}
			
		}
	}
	else if (foundAirLeft)
	{
		move = { x - 1, y };
		if (IsInside(move))
		{
			TileType t = GetTypeAt(move);
			if (IsMixable(GetTypeAt(x, y), t)) return MoveMix(x, y, move.x, move.y);
			else if (IsGas(t))
			{
				SwapTiles({ x, y }, move);
				return true;
			}
			
		}
	}
	else if (foundAirRight)
	{
		move = { x + 1, y };
		if (IsInside(move))
		{
			TileType t = GetTypeAt(move);
			if (IsMixable(GetTypeAt(x, y), t)) return MoveMix(x, y, move.x, move.y);
			else if (Empty(move) || IsGas(t))
			{
				SwapTiles({ x, y }, move);
				return true;
			}
		}
	}

	return false;
}

bool TileMap::SpreadRandom(uint32_t x, uint32_t y)
{
	bool startedFire = false;
	for (int32_t moveY = -1; moveY <= 1; ++moveY)
	{
		for (int32_t moveX = -1; moveX <= 1; ++moveX)
		{
			glm::ivec2 move{ x + moveX, y + moveY };
			if (IsInside(move))
			{
				TileType t = GetTypeAt(move);
				if (IsFlammable(t))
				{
					// If tile around is flammable, 
					// 10 % chance it catches fire
					if (rand() % 1000 < 5)
					{
						SetTile(move, TileType::Fire);
						startedFire = true;
						break;
					}
				}
			}

		}

		if (startedFire)
			break;
	}

	return startedFire;
}

bool TileMap::MoveMix(uint32_t sX, uint32_t sY, uint32_t mX, uint32_t mY)
{
	uint32_t topX{ 0 }, topY{ 0 };
	uint32_t bottomX{ 0 }, bottomY{ 0 };
	if (sY > mY)
	{
		topX = sX, topY = sY;
		bottomX = mX, bottomY = mY;
	}
	else
	{
		topY = mX, bottomY = mY;
		bottomX = sX, topY = sY;
	}

	m_TileData[CoordToIndex(topX, topY)] = DefaultTiles[TileType::Steam];
	m_TileData[CoordToIndex(bottomX, bottomY)] = DefaultTiles[TileType::Stone];

	return true;
}

const glm::vec4 TileMap::TileDataToColor(TileType bitData)
{
	switch (bitData)
	{
	case TileType::Air:     return { 0.000f, 0.000f, 0.000f, 1.0f };
	case TileType::Sand:    return { 0.760f, 0.698f, 0.502f, 1.0f };
	case TileType::Water:   return { 0.678f, 0.847f, 0.902f, 1.0f };
	case TileType::Lava:    return { 0.812f, 0.063f, 0.125f, 1.0f };
	case TileType::Steam:   return { 0.533f, 0.549f, 0.553f, 1.0f }; 
	case TileType::Smoke:   return { 0.961f, 0.961f, 0.961f, 0.1f }; 
	case TileType::Stone:   return { 0.533f, 0.549f, 0.553f, 0.1f };
	case TileType::Wood:    return { 0.572f, 0.526f, 0.271f, 1.0f };
	case TileType::Fire:    return { 0.886f, 0.345f, 0.133f, 1.0f };
	default:					 KB_ASSERT(false, "TILE NOT IMPLEMENTED");  return { 0.0f, 0.0f, 0.0f, 1.0f };
	}
}

