#pragma once

#include <Kablunk.h>

struct CameraController : public Kablunk::NativeScript
{
	CameraController()
	{

	}

	IMPLEMENT_NATIVE_SCRIPT(CameraController);

	void CameraController::OnUpdate(Kablunk::Timestep ts) override
	{
		if (Kablunk::Input::IsMouseButtonPressed(Kablunk::Mouse::ButtonLeft))
		{
			auto& camera_comp = GetComponent<Kablunk::CameraComponent>();
			auto& camera = camera_comp.Camera;

			auto& transform = GetComponent<Kablunk::TransformComponent>().GetTransform();
			auto [mx, my] = Kablunk::Input::GetMousePosition();
			auto mouse_pos = glm::vec2{ mx, my };
			auto screen_pos = camera.ScreenToWorldPosition2D(transform, mouse_pos);
			KB_CLIENT_TRACE("Mouse pos ({0}, {1})", screen_pos.x, screen_pos.y);
		}
	}
};
