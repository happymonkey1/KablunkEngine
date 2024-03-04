#pragma once

#include <lua.h>

#include "Kablunk/Core/Core.h"
#include "Kablunk/lua/lua_table_key.h"

#include <variant>


namespace kb::lua
{ // start namespace kb::lua

enum class lua_type_t : u8
{
    nil,
    boolean,
    number,
    string,
    function,
    // block of raw memory
    userdata,
    // #TODO: implement
    // thread,
    table
};

using lua_value_t = std::variant<
    // nil
    std::monostate,
    // boolean
    bool,
    // number
    double,
    // string
    std::string,
    // function
    lua_CFunction,
    // userdata (block of raw memory)
    u8*,
    // #TODO thread

    // table
    kb::unordered_flat_map<std::string, lua_table_key>
>;

struct lua_type
{
    lua_type_t m_type = lua_type_t::nil;
    lua_value_t m_value{};
};

} // end namespace kb::lua
