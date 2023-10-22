#ifndef KABLUNK_CORE_INPUT_H
#define KABLUNK_CORE_INPUT_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/KeyCodes.h"
#include "Kablunk/Core/MouseCodes.h"

namespace kb::input
{ // start namespace kb::input


// \brief return whether the passed in key code is pressed
// \warning not thread safe because of glfw
bool is_key_pressed(int keycode);
	
// \brief return whether the passed in mouse code is pressed
// \warning not thread safe because of glfw
bool is_mouse_button_pressed(int button);

// \brief check whether the mouse is inside the viewport
// usually used when the editor is running because the viewport is not fullscreen
bool is_mouse_in_viewport();

// \brief get mouse position relative to viewport
// usually used when the eidtor is running because the viewport is not fullscreen
std::pair<float, float> get_mouse_position_relative_to_viewport();

// \brief get mouse x relative to viewport
// usually used when the eidtor is running because the viewport is not fullscreen
float get_mouse_x_relative_to_viewport();

// \brief get mouse y relative to viewport
// usually used when the eidtor is running because the viewport is not fullscreen
float get_mouse_y_relative_to_viewport();

// \brief return mouse position within the application
// \warning not thread safe because of glfw
std::pair<float, float> get_mouse_position();
	
// \brief return mouse x position within the application
// \warning not thread safe because of glfw
float get_mouse_x();

// \brief return mouse y position within the application
// \warning not thread safe because of glfw
float get_mouse_y();

// \brief set the cursor mode
// \warning not thread safe because of glfw
void set_cursor_mode(CursorMode mode);

// \brief get the cursor mode
// \warning not thread safe because of glfw
CursorMode get_cursor_mode();

// \brief set the mouse motion mode
// \warning not thread safe because of glfw
void set_mouse_motion_mode(MouseMotionMode mode);

// \brief get the mouse motion mode
// \warning not thread safe because of glfw
MouseMotionMode get_mouse_motion_mode();

} // end namespace kb::input

#endif
