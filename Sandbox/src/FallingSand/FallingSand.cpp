#include "FallingSand/FallingSand.h"


FallingSand::FallingSand()
	: Layer("FallingSand"), m_TileMap{100, 100}, m_CameraController{ 1.7778f }
{
	m_CameraController.SetTranslationInputLocked(true);
	m_CameraController.SetScalingInputLocked(true);
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
				(int)(x - simStartX) / (int)m_TileMap.GetTileWidth(),
				(int)(y) / (int)m_TileMap.GetTileHeight()
			};

			m_TileMap.TrySetTile(position, TileMap::TILE_BIT_DATA::Sand);
			
		}
	}

	if (m_TickCounter >= m_TicksPerSecond)
	{
		m_TileMap.UpdateAllTiles();
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

	float tileWidth = m_TileMap.GetTileWidth(), tileHeight = m_TileMap.GetTileHeight();
	glm::vec2 size{ tileWidth / m_TileMap.GetSimulationWidthInPixels(), tileHeight / m_TileMap.GetSimulationHeightInPixels() };
	{
		KB_PROFILE_SCOPE("Rendering - FallingSand::OnUpdate()");
		
		for (auto it = m_TileMap.Begin(), end = m_TileMap.End(); it != end; ++it)
		{
			TileMap::Tile t = *it;
			glm::vec2 worldPosition = {
				(float)t.Position.x * tileWidth  * (size.x / 7.0f),
				(float)t.Position.y * tileHeight * (size.y / 7.0f)
			};

			Kablunk::Renderer2D::DrawQuad(worldPosition, size, t.Color);
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
	glm::vec2 size{ simWidth / m_TileMap.GetTileWidth() / 12.0f, simHeight / m_TileMap.GetTileHeight() / 12.0f};

	int worldX = ((camPos.x + (camAspectRatio) * ((float)x / (float)m_ScreenWidth) + (simWidth / 2.0f)) * size.x ) / m_TileMap.GetTileWidth();
	int worldY = (camPos.x + (camAspectRatio) * ((float)x / (float)m_ScreenWidth)) * size.y / m_TileMap.GetTileHeight();
	return (worldX > 0 && worldX < simWidth && worldY > 0 && worldY < simHeight);
}

