#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/lua/lua_error.h"

#include <tl/expected.hpp>

#include <variant>


namespace kb::lua
{ // start namespace kb::lua

enum class lua_table_key_type_t
{
    nil,
    number,
    string
};

enum class lua_table_key_access_error : u8{};

// wrapper for lua table value, since lua tables can contain numbers or strings
class lua_table_key
{
public:
    using lua_table_key_value_t = std::variant<f64, std::string>;
public:
    lua_table_key() noexcept = default;
    lua_table_key(const lua_table_key&) noexcept = default;
    lua_table_key(lua_table_key&&) noexcept = default;
    ~lua_table_key() noexcept = default;

    lua_table_key(lua_table_key_type_t p_type, lua_table_key_value_t p_value) noexcept
        : m_type{ p_type }, m_value{ p_value }
    { }

    [[nodiscard]] auto holds_number() const noexcept -> bool { return m_type == lua_table_key_type_t::number; }
    [[nodiscard]] auto holds_string() const noexcept -> bool { return m_type == lua_table_key_type_t::string; }

    auto operator<=>(const lua_table_key&) const -> std::partial_ordering = default;

    [[nodiscard]] tl::expected<f64, lua_error_t> get_number() const noexcept -> f64
    {
#ifdef KB_DEBUG
        // #TODO print enum string instead of value
        KB_CORE_ASSERT(m_type == lua_table_key_type_t::number, "[lua_table_key]: tried getting the value as a number, but contains {} instead!", m_type);
#endif

        return m_type == lua_table_key_type_t::number ? std::get<f64>(m_value) : tl::unexpected{ lua_error_t::internal_key_access_error };
    }

    [[nodiscard]] tl::expected<std::string, lua_error_t> get_string() const noexcept -> f64
    {
#ifdef KB_DEBUG
        // #TODO print enum string instead of value
        KB_CORE_ASSERT(m_type == lua_table_key_type_t::string, "[lua_table_key]: tried getting the value as a string, but contains {} instead!", m_type);
#endif

        return m_type == lua_table_key_type_t::string ? std::get<std::string>(m_value) : tl::unexpected{ lua_error_t::internal_key_access_error };
    }

    auto operator=(const lua_table_key&) noexcept -> lua_table_key& = default;
    auto operator=(lua_table_key&&) noexcept -> lua_table_key& = default;
private:
    lua_table_key_type_t m_type = lua_table_key_type_t::nil;
    lua_table_key_value_t m_value{};
};

} // end namespace kb::lua
