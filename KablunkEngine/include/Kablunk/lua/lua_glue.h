#pragma once

#include "Kablunk/lua/lua_core.h"

namespace kb::lua::glue
{ // start namespace kb

extern "C"
{

auto logger_info(lua_State* p_lua_state) -> int;


// ============
//   Renderer
// ============

auto draw_quad() -> void;
}

} // end namespace kb
