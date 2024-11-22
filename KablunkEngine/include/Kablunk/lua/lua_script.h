#pragma once

#include <sol/sol.hpp>

#include <filesystem>

namespace kb::lua
{ // start namespace kb::lua

struct lua_script
{
    std::string m_debug_name;
    sol::load_result m_script_data;
};

} // end namespace kb::lua
