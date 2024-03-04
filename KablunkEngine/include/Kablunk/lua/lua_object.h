#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/lua/lua_core.h"

namespace kb::lua
{ // start namespace kb::lua

// #TODO fix
using lua_type_t = u32;

// CRTP interface to for c++ interop with lua engine
template <typename DerivedT>
class lua_object
{
public:
    // get the value for the `lua_object` by index
    auto get_value(u32 p_key) noexcept -> std::shared_ptr<lua_type_t> { return static_cast<DerivedT>(this).get_value_impl(p_key); }
    // get the value for the `lua_object` by string
    auto get_value(const std::string& p_key) noexcept -> std::shared_ptr<lua_type_t> { return static_cast<DerivedT>(this).get_value_impl(p_key); }
    // set the value for the `lua_object` by index
    auto set_value(u32 p_key, std::shared_ptr<lua_type_t> p_value) { return static_cast<DerivedT>(this).set_value_impl(p_key, p_value); }
    // set the value for the `lua_object` by string
    auto set_value(const std::string& p_key, std::shared_ptr<lua_type_t> p_value) { return static_cast<DerivedT>(this).set_value_impl(p_key, p_value); }
};

} // end namespace kb::lua
