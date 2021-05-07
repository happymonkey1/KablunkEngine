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
		Water = 2,
		Lava = 3,
		Steam = 4,
		Smoke = 5,
		Stone = 6,
		Wood = 7,
		Fire = 8
	};

	static constexpr uint32_t TILES_LIST[9] { 
		TILE_BIT_DATA::Air, TILE_BIT_DATA::Sand, TILE_BIT_DATA::Water, 
		TILE_BIT_DATA::Lava, TILE_BIT_DATA::Steam, TILE_BIT_DATA::Smoke, 
		TILE_BIT_DATA::Stone, TILE_BIT_DATA::Wood, TILE_BIT_DATA::Fire 
	};

	struct Tile
	{
		glm::ivec2 Position;
		glm::vec4 Color;

		Tile(const glm::ivec2& p, const glm::vec4& c) : Position{ p }, Color{ c } { };
	};

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
				

				m_CurrentTile++;
				m_Index++;

				if (m_Index >= m_Rows * m_Cols)
				{
					m_CurrentTile = nullptr;
					m_Index = -1;
					return *this;
				}
			}

			
			return *this;
		}

		bool operator==(const ConstIterator& other) { 
			
			if (m_Index == -1 && other.m_Index == -1)
				return true;
			else if (m_Index != other.m_Index)
				return false;
			else
				return *m_CurrentTile == *other.m_CurrentTile;
			
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
	void Reset();

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
	
	TILE_BIT_DATA At(const glm::ivec2& pos) { return At(pos.x, pos.y); }
	TILE_BIT_DATA At(uint32_t x, uint32_t y);

	void SetTile(const glm::ivec2& pos, TILE_BIT_DATA data) { SetTile(pos.x, pos.y, data); }
	void SetTile(uint32_t x, uint32_t y, TILE_BIT_DATA data);

	bool TrySetTile(const glm::ivec2& pos, TILE_BIT_DATA data) { return TrySetTile(pos.x, pos.y, data); }
	bool TrySetTile(uint32_t x, uint32_t y, TILE_BIT_DATA data);

	// TODO: Create Iterators
	ConstIterator Begin() { return ConstIterator{ *this }; }
	ConstIterator End() { return ConstIterator{ TileMap{} }; }

	bool IsGas(TILE_BIT_DATA data) { return data == Steam || data == Smoke || data == Air; }
	bool IsLiquid(TILE_BIT_DATA data) { return data == Water || data == Lava; }
	bool IsSolid(TILE_BIT_DATA data) { return data == Sand || data == Stone; }
	bool IsFlammable(TILE_BIT_DATA data) { return data == Wood; }

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