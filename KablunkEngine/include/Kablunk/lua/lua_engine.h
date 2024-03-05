#pragma once

#include "Kablunk/lua/lua_core.h"

#include <string_view>

#include "Kablunk/Asset/AssetManager.h"
#include "Kablunk/lua/lua_asset.h"

namespace kb
{ // start namespace kb

// #TODO run on separate thread
class lua_engine
{
public:
    lua_engine() noexcept = default;
    lua_engine(const lua_engine&) noexcept = delete;
    lua_engine(lua_engine&&) noexcept = default;
    ~lua_engine() noexcept;

    lua_engine(const ref<asset::AssetManager>& p_asset_manager) noexcept
        : m_lua_state{ nullptr }, m_asset_manager{ p_asset_manager }
    { }

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

    auto operator=(const lua_engine&) noexcept -> lua_engine& = delete;
    auto operator=(lua_engine&&) noexcept -> lua_engine& = default;

private:
    auto register_engine_glue_functions() noexcept -> void;
    auto destroy() noexcept -> void;

private:
    lua_State* m_lua_state = nullptr;
    ref<asset::AssetManager> m_asset_manager{};
    // cache of lua assets that have been loaded into the engine
    std::vector<ref<lua_asset>> m_lua_scripts{};
};

} // end namespace kb
