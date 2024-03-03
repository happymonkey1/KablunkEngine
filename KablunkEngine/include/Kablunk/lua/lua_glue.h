#pragma once

#include "Kablunk/lua/lua_core.h"

namespace kb::lua
{ // start namespace kb

extern "C"
{
    auto logger_info(lua_State* p_lua_state) -> int;

}

} // end namespace kb
