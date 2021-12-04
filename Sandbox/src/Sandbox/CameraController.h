#pragma once

#include <Kablunk.h>
#if KB_NATIVE_SCRIPTING
struct CameraController : public Kablunk::NativeScript
{
	CameraController()
	{

	}

	//IMPLEMENT_NATIVE_SCRIPT(CameraController);

	void CameraController::OnUpdate(Kablunk::Timestep ts) override
	{
		if (Kablunk::Input::IsMouseButtonPressed(Kablunk::Mouse::ButtonLeft))
		{
			auto& camera_comp = GetComponent<Kablunk::CameraComponent>();
			auto& camera = camera_comp.Camera;

			auto& transform = GetComponent<Kablunk::TransformComponent>().GetTransform();
			auto screen_pos = camera.ScreenToWorldPosition2D(transform);
			KB_CLIENT_TRACE("Mouse pos ({0}, {1})", screen_pos.x, screen_pos.y);
		}
	}
};
#endif
