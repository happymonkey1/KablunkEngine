#ifndef KABLUNK_CORE_INPUT_H
#define KABLUNK_CORE_INPUT_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/KeyCodes.h"
#include "Kablunk/Core/MouseCodes.h"

namespace Kablunk {

	class Input {
	public:
		static bool IsKeyPressed(int keycode);
		
		static bool IsMouseButtonPressed(int button);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}

#endif