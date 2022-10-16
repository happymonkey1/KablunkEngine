#ifndef KABLUNK_CORE_INPUT_H
#define KABLUNK_CORE_INPUT_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/KeyCodes.h"
#include "Kablunk/Core/MouseCodes.h"

namespace Kablunk::input
{
	// \brief return whether the passed in key code is pressed
	bool is_key_pressed(int keycode);
	
	// \brief return whether the passed in mouse code is pressed
	bool is_mouse_button_pressed(int button);

	// \brief return mouse position within the application
	std::pair<float, float> get_mouse_position();
	
	// \brief return mouse x position within the application
	float get_mouse_x();

	// \brief return mouse y position within the application
	float get_mouse_y();

	// \brief set the cursor mode
	void set_cursor_mode(CursorMode mode);

	// \brief get the cursor mode
	CursorMode get_cursor_mode();

	// \brief set the mouse motion mode
	void set_mouse_motion_mode(MouseMotionMode mode);

	// \brief get the mouse motion mode
	MouseMotionMode get_mouse_motion_mode();
}

#endif
