#include "kablunkpch.h"
#include "Kablunk/lua/lua_engine.h"

#include "Kablunk/lua/lua_glue.h"
#include "Kablunk/Renderer/renderer_2d.h"

namespace kb::lua
{ // start namespace kb::lua


lua_engine::~lua_engine() noexcept
{
    destroy();
}

auto lua_engine::init() noexcept -> void
{
    KB_CORE_INFO("Initializing lua engine");

    // luaL_openlibs(m_lua_state);
    m_lua_state.open_libraries(sol::lib::base, sol::lib::package);

    register_engine_glue_functions();

    // TODO: load engine lua scripts

    KB_CORE_INFO(
        "[lua_engine]: Cached {} lua scripts to run in the engine",
        m_lua_scripts.size()
    );

    KB_CORE_ASSERT(m_lua_state.lua_state(), "[lua_engine]: lua state is null?");
    KB_CORE_INFO("Finished initializing lua engine");
}

auto lua_engine::load_file(const std::string& p_script_path, std::string p_debug_name) noexcept -> lua_script_handle
{
    sol::load_result loaded_script = m_lua_state.load_file(p_script_path);
    KB_CORE_ASSERT(loaded_script.valid(), "[lua_engine]: Failed to load lua script '{}'", p_script_path);

    const auto script_handle = lua_script_handle::into(p_script_path);

    m_lua_scripts.emplace(
        script_handle,
        lua_script{
            .m_debug_name = std::move(p_debug_name),
            .m_script_data = std::move(loaded_script)
        }
    );
    return script_handle;
}

auto lua_engine::run_file(const std::filesystem::path& p_script_path) noexcept -> void
{
    KB_CORE_ASSERT(m_lua_state.lua_state(), "Tried to run Lua script, but Lua state is not initialized!");

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

auto lua_engine::run_script(lua_script_handle p_script_handle) noexcept -> option<lua_error_t>
{
    if (!m_lua_scripts.contains(p_script_handle))
    {
        KB_CORE_WARN("[lua_engine]: Could not find lua script with handle={}", p_script_handle.as<u32>());
        return lua_error_t::script_not_found;
    }

    auto& script = m_lua_scripts.at(p_script_handle);

    const auto result = script.m_script_data();
    if (!result.valid())
    {
        KB_CORE_ERROR("[lua_engine]: Error while running '{}'", script.m_debug_name);
        return lua_error_t::script_error;
    }

    return std::nullopt;
}

auto lua_engine::add_renderer_2d_reference(
    std::string_view p_name,
    weak_ptr<render::renderer_2d> p_renderer_2d
) noexcept -> void
{
    auto* renderer_2d = p_renderer_2d.get();
    m_lua_state[p_name] = renderer_2d;
    sol::table lua_renderer_2d = m_lua_state.get<>();

    // Member function pointer decl for the overload we want
    using draw_quad_base_func_t = void (render::renderer_2d::*)(
        const glm::vec3&,
        const glm::vec2&,
        const arc<render::backend::texture_2d>&,
        f32,
        const glm::vec4&
    );
    draw_quad_base_func_t p_draw_quad_func = &render::renderer_2d::draw_quad;
    lua_renderer_2d.set_function("draw_quad", p_draw_quad_func);
}

auto lua_engine::register_engine_glue_functions() noexcept -> void
{
    KB_CORE_INFO("[lua_engine]: Starting to register engine glue functions");

    // register_glue_function_old(lua::logger_info, "logger_info");
    register_glue_function("logger_info", glue::logger_info);

    KB_CORE_INFO("[lua_engine]: Finished registering engine glue functions");
}

auto lua_engine::destroy() noexcept -> void
{
    KB_CORE_INFO("[lua_engine]: Destroying lua engine");
    m_lua_state = nullptr;

    m_lua_scripts.clear();
}

} // end namespace kb::lua
