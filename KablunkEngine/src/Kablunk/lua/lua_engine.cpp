#include "kablunkpch.h"
#include "Kablunk/lua/lua_engine.h"
#include "Kablunk/lua/lua_glue.h"

namespace kb
{ // start namespace kb


lua_engine::~lua_engine()
{
    KB_CORE_INFO("[lua_engine]: Shutting down");
    if (m_lua_state)
        lua_close(m_lua_state);
}

auto lua_engine::init() -> void
{
    KB_CORE_INFO("Initializing lua engine");

    m_lua_state = luaL_newstate();
    luaL_openlibs(m_lua_state);

    add_glue_function(lua::logger_info, "logger_info");

    KB_CORE_ASSERT(m_lua_state, "[lua_engine]: lua state is null?");
    KB_CORE_INFO("Finished initializing lua engine");
}

auto lua_engine::shutdown() -> void
{
}

auto lua_engine::run_file(const std::filesystem::path& p_script_path) -> void
{
    KB_CORE_ASSERT(m_lua_state, "Tried to run Lua script, but Lua state is not initialized!");

    auto status = luaL_dofile(m_lua_state, p_script_path.string().c_str());
    if (status == LUA_OK)
    {
        lua_pop(m_lua_state, lua_gettop(m_lua_state));
    }
    else
    {
        KB_CORE_WARN("[lua_engine]: Tried running '{}' but something went wrong!", p_script_path.string().c_str());
        KB_CORE_WARN("[lua_engine]:   lua error: {}", lua_tostring(m_lua_state, lua_gettop(m_lua_state)));
        lua_pop(m_lua_state, lua_gettop(m_lua_state));
    }
}

} // end namespace kb
