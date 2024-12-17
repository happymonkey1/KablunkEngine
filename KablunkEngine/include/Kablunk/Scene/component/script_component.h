#pragma once

#include <string>

#include "Kablunk/lua/lua_script_handle.h"

namespace kb
{ // start namespace kb

struct lua_script_component
{
    std::string m_file_path;
    lua_script_handle m_handle;
};

} // end namespace kb
