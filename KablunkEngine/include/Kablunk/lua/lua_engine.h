#pragma once

#include "Kablunk/lua/lua_core.h"

#include <string_view>

namespace kb
{ // start namespace kb

// #TODO run on separate thread
class lua_engine
{
public:
    lua_engine() noexcept = default;
    ~lua_engine() noexcept;

    auto init() noexcept -> void;
    auto shutdown() noexcept -> void;

    auto run_file(const std::filesystem::path& p_script_path) noexcept -> void;

    // expose an engine function to the lua vm
    // NOTE: function must be marked as `extern c`
    auto register_glue_function(lua_CFunction p_c_func, std::string_view p_lua_func_name) noexcept -> void
    {
        KB_CORE_ASSERT(m_lua_state, "[lua_engine]: Trying to add engine function but lua state is not initialized!");

        lua_pushcfunction(m_lua_state, p_c_func);
        // exposes the value at the top of the lua stack as a global
        lua_setglobal(m_lua_state, p_lua_func_name.data());
    }
private:
    lua_State* m_lua_state = nullptr;
};

} // end namespace kb
