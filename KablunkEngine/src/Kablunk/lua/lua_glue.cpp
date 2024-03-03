#include "kablunkpch.h"
#include "Kablunk/lua/lua_glue.h"

#include <string_view>

namespace kb::lua
{ // start namespace kb

auto logger_info(lua_State* p_lua_state) -> int
{
    size_t message_size = 0;
    const char* lua_string = luaL_checklstring(p_lua_state, 1, &message_size);

    std::string_view message{ lua_string, message_size };

    KB_CORE_INFO("[lua_glue]: lua script says {}", message);

    return 0;
}

} // end namespace kb
