#ifndef FALLINGSAND_TILEMAP_H
#define FALLINGSAND_TILEMAP_H

#include <glm/glm.hpp>
#include <unordered_map>



enum class TileType
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

enum class MoveProps
{
	None = 0b00000000,
	MoveDown = 0b00000001,
	MoveDownSide = 0b00000010,
	MoveSide = 0b00000100,
	Gas = 0b00001000,
	SpreadRandom = 0b00010000
};

inline MoveProps operator|(MoveProps a, MoveProps b) { return MoveProps(int(a) | int(b)); }
inline auto operator&(MoveProps a, MoveProps b) { return int(a) & int(b); }


class TileMap
{
public:
	static constexpr TileType TILES_LIST[9] { 
		TileType::Air, TileType::Sand, TileType::Water, 
		TileType::Lava, TileType::Steam, TileType::Smoke, 
		TileType::Stone, TileType::Wood, TileType::Fire 
	};

	struct Tile {
		TileType Type{ TileType::Air };
		MoveProps MoveProperties{ MoveProps::None };

		glm::vec4 Color;

		bool operator==(const Tile& t) const { return Type == t.Type; }
		bool operator==(TileType t) const { return Type == t; }
	};

	struct TileEntity
	{
		glm::ivec2 Position;
		glm::vec4 Color;

		TileEntity(const glm::ivec2& p, const glm::vec4& c) : Position{ p }, Color{ c } { };
	};

	class ConstIterator
	{
	public:
		ConstIterator(const TileMap& tileMap)
		{
			if (tileMap.m_TileCols == 0 || tileMap.m_TileRows == 0)
			{
				m_CurrentTile = nullptr;
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
				m_CurrentTile = nullptr;
				m_Index = -1;
				return *this;
			}

			while (*m_CurrentTile == TileType::Air && m_Index != -1)
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
		TileEntity operator*() const { return TileEntity{ glm::ivec2{m_Index % m_Rows, m_Index / m_Cols}, TileDataToColor(m_CurrentTile->Type) };};
	private:
		Tile* m_CurrentTile;

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

	int32_t GetRows() const { return m_TileRows; }
	int32_t GetCols() const { return m_TileCols; }

	float GetTileWidth() const { return m_TileWidth; }
	float GetTileHeight() const { return m_TileHeight; }
	glm::vec2 GetTileSize() const { return {m_TileWidth, m_TileHeight}; }

	float GetSimulationWidthInPixels() { return m_SimulationWidth; }
	float GetSimulationHeightInPixels() { return m_SimulationHeight; }
	glm::vec2 GetSimulationResolution() const { return { m_SimulationWidth, m_SimulationHeight }; }

	

	uint32_t CoordToIndex(const glm::ivec2& pos) { return CoordToIndex(pos.x, pos.y); }
	uint32_t CoordToIndex(uint32_t x, uint32_t y) { return y * m_TileRows + x; }

	bool IsInside(const glm::ivec2& pos) { return IsInside(pos.x, pos.y); }
	bool IsInside(uint32_t x, uint32_t y) { return (x < m_TileRows && x >= 0 && y < m_TileCols && y >= 0); }

	bool Empty(const glm::ivec2& pos) { return Empty(pos.x, pos.y); }
	bool Empty(uint32_t x, uint32_t y);
	
	const Tile& At(const glm::ivec2& pos) { return At(pos.x, pos.y); }
	const Tile& At(uint32_t x, uint32_t y);

	TileType GetTypeAt(const glm::ivec2& pos) { return GetTypeAt(pos.x, pos.y); }
	TileType GetTypeAt(uint32_t x, uint32_t y) { return At(x, y).Type; }

	void SetTile(const glm::ivec2& pos, TileType data) { SetTile(pos.x, pos.y, data); }
	void SetTile(uint32_t x, uint32_t y, TileType data);

	bool TrySetTile(const glm::ivec2& pos, TileType data) { return TrySetTile(pos.x, pos.y, data); }
	bool TrySetTile(uint32_t x, uint32_t y, TileType data);

	// TODO: Create Iterators
	ConstIterator Begin() { return ConstIterator{ *this }; }
	ConstIterator End() { return ConstIterator{ TileMap{} }; }

	bool IsGas(TileType data) { return data == TileType::Steam || data == TileType::Smoke || data == TileType::Air; }
	bool IsLiquid(TileType data) { return data == TileType::Water || data == TileType::Lava; }
	bool IsSolid(TileType data) { return data == TileType::Sand || data == TileType::Stone; }
	bool IsFlammable(TileType data) { return data == TileType::Wood; }

private:
	Tile* m_TileData;

	int32_t m_TileRows{ 0 };
	int32_t m_TileCols{ 0 };

	float m_SimulationWidth{ 700.0f };
	float m_SimulationHeight{ 700.0f };

	float m_TileWidth;
	float m_TileHeight;

	std::unordered_map<TileType, TileMap::Tile> DefaultTiles{
		{ TileType::Air, {TileType::Air, MoveProps::None} },
		{ TileType::Stone, {TileType::Stone, MoveProps::None} },
		{ TileType::Wood, {TileType::Wood, MoveProps::None} },
		{ TileType::Sand, {TileType::Sand, MoveProps::MoveDown | MoveProps::MoveDownSide} },
		{ TileType::Water, {TileType::Water, MoveProps::MoveDown | MoveProps::MoveDownSide | MoveProps::MoveSide} },
		{ TileType::Lava, {TileType::Lava, MoveProps::MoveDown | MoveProps::MoveDownSide | MoveProps::MoveSide} },
		{ TileType::Steam, {TileType::Steam, MoveProps::MoveDown | MoveProps::MoveDownSide | MoveProps::MoveSide | MoveProps::Gas} },
		{ TileType::Smoke, {TileType::Smoke, MoveProps::MoveDown | MoveProps::MoveDownSide | MoveProps::MoveSide | MoveProps::Gas} },
		{ TileType::Fire, {TileType::Fire, MoveProps::SpreadRandom} },
	};

private:
	bool UpdateTile(uint32_t x, uint32_t y, TileType bitData);
	static const glm::vec4 TileDataToColor(TileType bitData);
};



#endif