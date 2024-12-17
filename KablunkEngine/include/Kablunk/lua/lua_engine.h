#pragma once

#include "Kablunk/Asset/AssetManager.h"
#include "Kablunk/lua/lua_core.h" // Includes <sol/sol.hpp>
#include "Kablunk/lua/lua_asset.h"
#include "Kablunk/lua/lua_error.h"
#include "Kablunk/lua/lua_script.h"
#include "Kablunk/lua/lua_script_handle.h"

#include <string_view>


namespace kb::render
{ // start namespace kb::render
class renderer_2d;
} // end namespace kb::render

namespace kb::lua
{ // start namespace kb::lua

// #TODO run on separate thread
class lua_engine
{
public:
    lua_engine() noexcept = default;
    lua_engine(const lua_engine&) noexcept = delete;
    lua_engine(lua_engine&&) noexcept = default;
    ~lua_engine() noexcept;

    auto init() noexcept -> void;

    auto load_file(const std::string& p_script_path, std::string p_debug_name = "unnamed lua script") noexcept -> lua_script_handle;
    auto run_file(const std::filesystem::path& p_script_path) noexcept -> void;
    auto run_script(lua_script_handle p_script_handle) noexcept -> option<lua_error_t>;

    // expose an engine function to the lua vm
    // NOTE: function must be marked as `extern c`
    auto register_glue_function_old(lua_CFunction p_c_func, std::string_view p_lua_func_name) noexcept -> void
    {
        KB_CORE_ASSERT(m_lua_state.lua_state(), "[lua_engine]: Trying to add engine function but lua state is not initialized!");

        lua_pushcfunction(m_lua_state.lua_state(), p_c_func);
        // exposes the value at the top of the lua stack as a global
        lua_setglobal(m_lua_state.lua_state(), p_lua_func_name.data());
    }

    auto register_glue_function(std::string_view p_func_name, auto p_func) noexcept -> void
    {
        KB_CORE_ASSERT(!m_lua_state[p_func_name].valid(), "[lua_engine]: {} glue function is already registered!", p_func_name);
        m_lua_state[p_func_name] = p_func;
    }

    auto add_object_reference(std::string_view p_object_name, auto& p_object) noexcept -> void
    {
        m_lua_state[p_object_name] = &p_object;
    }

    auto add_renderer_2d_reference(
        std::string_view p_name,
        weak_ptr<render::renderer_2d> p_renderer_2d
    ) noexcept -> void;

    // Retrieve a proxy to a lua table (object) owned by the engine
    [[nodiscard]] auto get_object(std::string_view p_object_name) const noexcept -> decltype(auto)
    {
        return m_lua_state[p_object_name];
    }

    auto operator=(const lua_engine&) noexcept -> lua_engine& = delete;
    auto operator=(lua_engine&&) noexcept -> lua_engine& = default;

private:
    auto register_engine_glue_functions() noexcept -> void;
    auto destroy() noexcept -> void;

private:
    sol::state m_lua_state{};
    // cache of lua assets that have been loaded into the engine
    std::unordered_map<lua_script_handle, lua_script> m_lua_scripts{};
};

} // end namespace kb::lua
