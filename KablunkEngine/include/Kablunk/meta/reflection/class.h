#pragma once

#include "Kablunk/meta/reflection/member.h"

#include <array>
#include <fmt/format.h>

namespace kb::meta::refl
{ // start namespace kb::meta::refl

template <u32 MemberCount>
class refl_class
{
public:
    static constexpr auto k_member_count = MemberCount;

    constexpr refl_class() noexcept = default;
    constexpr ~refl_class() noexcept = default;

    // Get the size of the class in bytes
    [[nodiscard]] consteval auto get_size_in_bytes() const noexcept -> u32 { return m_size; }
    // Get the number of members the reflected class contains
    [[nodiscard]] static consteval auto get_member_count() noexcept -> u32 { return MemberCount; }
    // Get compile time member reflection data
    [[nodiscard]] consteval auto get_member_at(u32 p_member_index) const noexcept -> refl_member
    {
        return m_members.at(p_member_index);
    }

    // Get the class name
    [[nodiscard]] consteval auto get_name() const noexcept -> std::string_view { return m_name; }


    // TODO: seems as though std::format and fmt::format do not have a constexpr variant,
    //       should probably pull fmt changes, may have an update to fix
#if 0
    [[nodiscard]] consteval auto get_debug_string() const noexcept -> std::string
    {
        constexpr std::string_view k_members_as_string{ "" };

        return fmt::format("{0}{ {1} }", m_name, k_members_as_string);
    }
#endif

    // Constexpr ordering operator
    [[nodiscard]] constexpr auto operator<=>(const refl_class&) const = default;

private:
    // Private constructor for create function
    constexpr refl_class(
        std::string_view p_class_name,
        std::array<refl_member, MemberCount> p_members,
        u32 p_size
    ) noexcept :
        m_name{ p_class_name }, m_members{ p_members }, m_size{ p_size }
    {
    }

private:
    // Name of the class
    std::string_view m_name{};
    // Compile time list of members
    std::array<refl_member, MemberCount> m_members;
    // Size of the class
    u32 m_size;

    template <typename... Args>
    friend consteval auto reflect_class(
        std::string_view p_class_name,
        u32 p_size,
        Args&&... p_members
    ) noexcept -> refl_class<sizeof...(Args)>;
};

// TODO: move to separate header
namespace concepts
{ // start namespace ::concepts

template <typename T, u32 MemberCount>
concept ReflectedT = requires
{
    { T::get_reflected_class() } -> std::same_as<refl_class<MemberCount>>;
};

} // end namespace ::concepts

// TODO: concept to ensure type is reflected
template <u32 MemberCount, concepts::ReflectedT<MemberCount> T>
[[nodiscard]] consteval auto get_member(T&& p_object, u32 p_member_index) -> decltype(auto)
{
    const auto refl_data = T::get_reflected_class();
    using reflected_type = typename T::get_reflected_class();
    using MemberCount = reflected_type::k_member_count;
    return refl_data.get_member_at(p_member_index);
}

} // end namespace kb::meta::refl
