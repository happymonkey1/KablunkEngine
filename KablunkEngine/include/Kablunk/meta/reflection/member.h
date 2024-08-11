#pragma once

#include "Kablunk/meta/reflection/type.h"

namespace kb::meta::refl
{ // start namespace kb::meta::refl

class refl_member
{
public:
    // Compile time default constructor
    constexpr refl_member() noexcept = default;
    // Compile time default destructor
    constexpr ~refl_member() noexcept = default;

    // Factory create function
    template <typename T>
    [[nodiscard]] static consteval auto create(std::string_view p_name, u32 p_offset) noexcept -> refl_member
    {
        return refl_member{
            p_name,
            refl_type::create<T>(),
            p_offset
        };
    }

    // Get compile time reflection type data
    [[nodiscard]] consteval auto get_refl_type() const noexcept -> refl_type { return m_type; }

    // Get member offset
    [[nodiscard]] consteval auto get_offset() const noexcept -> u32 { return m_offset; }

    [[nodiscard]] constexpr auto operator<=>(const refl_member&) const = default;
private:
    // Private constructor for create function
    constexpr refl_member(
        std::string_view p_name,
        refl_type p_refl_type,
        u32 p_offset
    ) noexcept :
        m_name{ p_name }, m_type{ p_refl_type }, m_offset{ p_offset }
    { }

private:
    // Name of the member
    std::string_view m_name;
    // Compile time member data
    refl_type m_type;
    // Member offset in bytes
    u32 m_offset;
};

}
