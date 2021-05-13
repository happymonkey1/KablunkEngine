#include "FallingSand/FallingSand.h"


FallingSand::FallingSand()
	: Layer("FallingSand"), m_TileMap{100, 100}, m_CameraController{ 1.7778f }
{
	m_CameraController.SetTranslationInputLocked(true);
	m_CameraController.SetScalingInputLocked(true);

	glm::vec3 oldPos = m_CameraController.GetPosition();
	m_CameraController.SetZoomLevel(0.5f);
	m_CameraController.SetPosition({ 0.5f, 0.5f, oldPos.z });

	m_Frog1 = { {0.5f, 0.5f}, m_TileMap.GetTileSize() / m_TileMap.GetSimulationResolution() * 10.0f, {0.0f, 0.0f} };
}


void FallingSand::OnAttach()
{
	
}

void FallingSand::OnDetach()
{
	
}

void FallingSand::OnUpdate(Kablunk::Timestep ts)
{
	KB_PROFILE_FUNCTION();

	// ==========
	//   Update
	// ==========

	m_CameraController.OnUpdate(ts);

	float rows{ (float)m_TileMap.GetRows() }, cols{ (float)m_TileMap.GetCols()};
	
	float simStartX = (float)(m_ScreenWidth - m_TileMap.GetSimulationWidthInPixels()) / 2.0f;
	float simStartY = (float)(m_ScreenHeight - m_TileMap.GetSimulationHeightInPixels()) / 2.0f;
	if (Kablunk::Input::IsMouseButtonPressed(KB_MOUSE_BUTTON_1))
	{
		float x = Kablunk::Input::GetMouseX(), y = Kablunk::Input::GetMouseY();
		
		if (MouseInsideSimulation(x, y))
		{
			glm::ivec2 position = {
				(x - simStartX) / m_TileMap.GetTileWidth(),
				((static_cast<float>(m_ScreenHeight) - y) / m_TileMap.GetTileHeight()) - 1
			};

			for (int y = -m_BrushRadius; y <= m_BrushRadius; ++y)
				for (int x = -m_BrushRadius; x <= m_BrushRadius; ++x)
					m_TileMap.TrySetTile({position.x + x, position.y + y}, m_CurrentTileTypeSelected);
		}
	}

	if (m_TickCounter >= m_TicksPerSecond)
	{
		m_TileMap.UpdateAllTiles();
		

		m_Frog1.OnUpdate(Kablunk::Timestep{m_TickCounter});
		m_TickCounter -= m_TicksPerSecond;
	}
	else
		m_TickCounter += ts.GetMiliseconds() / 1000.0f;

	m_TileMap.FlagTilesForUpdate();

	// ==========
	//   Render
	// ==========

	
	
	Kablunk::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
	Kablunk::RenderCommand::Clear();

	Kablunk::Renderer2D::ResetStats();
	Kablunk::Renderer2D::BeginScene(m_CameraController.GetCamera());

	float tileWidth = m_TileMap.GetTileWidth(), tileHeight = m_TileMap.GetTileHeight();
	glm::vec2 size{ tileWidth / m_TileMap.GetSimulationWidthInPixels(), tileHeight / m_TileMap.GetSimulationHeightInPixels() };
	{
		KB_PROFILE_SCOPE("Rendering - FallingSand::OnUpdate()");
		
		for (auto it = m_TileMap.Begin(), end = m_TileMap.End(); it != end; ++it)
		{
			TileMap::TileEntity t = *it;
			glm::vec2 worldPosition = {
				(float)t.Position.x * tileWidth  * (size.x / (m_TileMap.GetSimulationWidthInPixels() / static_cast<float>(m_TileMap.GetRows()) )),
				(float)t.Position.y * tileHeight * (size.y / (m_TileMap.GetSimulationHeightInPixels() / static_cast<float>(m_TileMap.GetCols() )))
			};

			Kablunk::Renderer2D::DrawQuad(worldPosition, size, t.Color);
		}
	}
	//Kablunk::Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 1.0f,1.0f }, TileDataToColor(TILE_BIT_DATA::Sand));

	m_Frog1.OnRender();

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

	Kablunk::Renderer2D::Renderer2DStats stats = Kablunk::Renderer2D::GetStats();

	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Verts: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
	ImGui::Text("Quad Count: %d", stats.QuadCount);

	ImGui::End();

	ImGui::Begin("Brush Settings");

	const char* tiles[] = { "Air", "Sand", "Water", "Lava", "Steam", "Smoke", "Stone", "Wood", "Fire"};
	static const char* currentItem = tiles[1];
	if (ImGui::BeginCombo("##Tiles", currentItem))
	{
		size_t s = sizeof(tiles) / sizeof(char*);
		for (int i = 0; i < s; ++i)
		{
			bool isSelected = currentItem == tiles[i];
			if (ImGui::Selectable(tiles[i], isSelected))
			{
				currentItem = tiles[i];
				m_CurrentTileTypeSelected = static_cast<TileType>(TileMap::TILES_LIST[i]);
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	const char* brushNames[] = { "1", "2", "5", "10", "20", "50", "100"};
	int brushSizes[] = { 1, 2, 5, 10, 20, 50, 100 };
	static const char* currentBrushSize = brushNames[0];
	std::string brushSizeTitle = "Brush size: " + std::to_string(m_BrushRadius);
	if (ImGui::BeginCombo(brushSizeTitle.c_str(), currentBrushSize, ImGuiComboFlags_NoPreview))
	{
		size_t s = sizeof(brushNames) / sizeof(char*);
		for (int i = 0; i < s; ++i)
		{
			bool isSelected = currentBrushSize == brushNames[i];
			if (ImGui::Selectable(brushNames[i], isSelected))
			{
				currentBrushSize = brushNames[i];
				m_BrushRadius = brushSizes[i];
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Reset Map"))
	{
		m_TileMap.Reset();
	}

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

	KB_CLIENT_INFO("Window resize called");
	return false;
}

bool FallingSand::MouseInsideSimulation(const glm::ivec2& pos)
{
	return MouseInsideSimulation(pos.x, pos.y);
}

bool FallingSand::MouseInsideSimulation(uint32_t x, uint32_t y)
{
	const glm::vec3& camPos = m_CameraController.GetPosition();
	float camAspectRatio = m_CameraController.GetAspectRatio();

	

	float simWidth = m_TileMap.GetSimulationWidthInPixels(), simHeight = m_TileMap.GetSimulationHeightInPixels();
	float simStartX = (m_ScreenWidth - simWidth) / 2.0f, simStartY = (m_ScreenHeight - simHeight) / 2.0f;

	return (x >= simStartX && x < simStartX + simWidth && y >= simStartY && y < simStartY + simHeight);
}

