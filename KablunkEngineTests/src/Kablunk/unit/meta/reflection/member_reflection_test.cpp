#include "Kablunk/meta/reflection/member.h"

#include <catch_amalgamated.hpp>

#include <cstddef>

using namespace kb;
using namespace kb::meta;

TEST_CASE("basic constexpr reflected member", "[meta::refl]")
{
    struct foo_t
    {
        u32 m_a;
        u32 m_b;
    };

    constexpr auto foo = foo_t{};
    constexpr auto foo_reflected_member_a = refl::refl_member::create<decltype(foo.m_a)>("m_a", offsetof(foo_t, m_a));
    constexpr auto foo_reflected_member_b = refl::refl_member::create<decltype(foo.m_a)>("m_b", offsetof(foo_t, m_b));

    constexpr auto foo_reflected_member_type = foo_reflected_member_a.get_refl_type();

    STATIC_REQUIRE(foo_reflected_member_type.get_type() == refl::type_t::u32);
    STATIC_REQUIRE(foo_reflected_member_type.is_const() == false);

    STATIC_REQUIRE(foo_reflected_member_a.get_offset() == 0);
    STATIC_REQUIRE(foo_reflected_member_b.get_offset() == 4);
}
