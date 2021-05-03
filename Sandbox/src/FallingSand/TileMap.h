#ifndef FALLINGSAND_TILEMAP_H
#define FALLINGSAND_TILEMAP_H

#include <glm/glm.hpp>

class TileMap
{
public:
	enum TILE_BIT_DATA
	{
		Air = 0,
		Sand = 1,
		Water = 2
	};

	struct Tile
	{
		glm::ivec2 Position;
		glm::vec4 Color;

		Tile(const glm::ivec2& p, const glm::vec4& c) : Position{ p }, Color{ c } { };
	};

public:
	class ConstIterator
	{
	public:
		ConstIterator(const TileMap& tileMap)
		{
			if (tileMap.m_TileCols == 0 || tileMap.m_TileRows == 0)
			{
				m_CurrentTile == nullptr;
				m_Index = -1;
			}
			else
			{
				m_CurrentTile = tileMap.m_TileData;
				m_Rows = tileMap.m_TileRows;
				m_Cols = tileMap.m_TileCols;
			}
			
		}

		const ConstIterator& operator++()
		{
			if (m_Index == -1)
				return *this;


			m_CurrentTile++;
			m_Index++;

			if (m_Index >= m_Rows * m_Cols)
			{
				m_CurrentTile == nullptr;
				m_Index = -1;
				return *this;
			}

			while (*m_CurrentTile == TILE_BIT_DATA::Air && m_Index != -1)
			{
				if (m_Index >= m_Rows * m_Cols)
				{
					m_CurrentTile = nullptr;
					m_Index == -1;
					return *this;
				}

				m_CurrentTile++;
				m_Index++;
			}

			
			return *this;
		}

		bool operator==(const ConstIterator& other) { 
			
			if (m_Index != other.m_Index)
				return false;
			else
			{
				if (m_Index == -1)
					return true;

				return *m_CurrentTile == *other.m_CurrentTile;
			}
		}
		
		bool operator!=(const ConstIterator& other) { return !(*this == other); }
		Tile operator*() const { return Tile{ glm::ivec2{m_Index % m_Rows, m_Index / m_Cols}, TileDataToColor(*m_CurrentTile) };};
	private:
		TILE_BIT_DATA* m_CurrentTile;

		int m_Index;
		int m_Rows;
		int m_Cols;
	};

public:
	TileMap();
	TileMap(int rows, int cols);
	~TileMap();

	void InitTileData();
	void UpdateAllTiles();

	int32_t GetRows() { return m_TileRows; }
	int32_t GetCols() { return m_TileCols; }

	float GetTileWidth() { return m_TileWidth; }
	float GetTileHeight() { return m_TileHeight; }

	float GetSimulationWidthInPixels() { return m_SimulationWidth; }
	float GetSimulationHeightInPixels() { return m_SimulationHeight; }

	uint32_t CoordToIndex(const glm::ivec2& pos) { return CoordToIndex(pos.x, pos.y); }
	uint32_t CoordToIndex(uint32_t x, uint32_t y) { return y * m_TileRows + x; }

	bool IsInside(const glm::ivec2& pos) { return IsInside(pos.x, pos.y); }
	bool IsInside(uint32_t x, uint32_t y) { return (x < m_TileRows && x >= 0 && y < m_TileCols && y >= 0); }

	bool Empty(const glm::ivec2& pos) { return Empty(pos.x, pos.y); }
	bool Empty(uint32_t x, uint32_t y);
	

	void SetTile(const glm::ivec2& pos, TILE_BIT_DATA data) { SetTile(pos.x, pos.y, data); }
	void SetTile(uint32_t x, uint32_t y, TILE_BIT_DATA data);

	bool TrySetTile(const glm::ivec2& pos, TILE_BIT_DATA data) { return TrySetTile(pos.x, pos.y, data); }
	bool TrySetTile(uint32_t x, uint32_t y, TILE_BIT_DATA data);

	// TODO: Create Iterators
	ConstIterator Begin() { return ConstIterator{ *this }; }
	ConstIterator End() { return ConstIterator{ TileMap{} }; }



private:
	TILE_BIT_DATA* m_TileData;

	int32_t m_TileRows{ 0 };
	int32_t m_TileCols{ 0 };

	float m_SimulationWidth{ 700.0f };
	float m_SimulationHeight{ 700.0f };

	float m_TileWidth;
	float m_TileHeight;

private:
	bool UpdateTile(uint32_t x, uint32_t y, TILE_BIT_DATA bitData);
	static const glm::vec4 TileDataToColor(TILE_BIT_DATA bitData);



};

#endif