#include "Kablunk/meta/reflection/class.h"

#include <catch_amalgamated.hpp>

#include "Kablunk/meta/reflection/reflect.h"

using namespace kb;
using namespace kb::meta;

TEST_CASE("basic constexpr reflected class", "[meta::refl]")
{
    struct foo_t
    {
        u32 m_a;
        u32 m_b;

        KB_REFLECT_CLASS_START(foo_t)
        KB_REFLECT_MEMBER(m_a),
        KB_REFLECT_MEMBER(m_b)
        KB_REFLECT_CLASS_END
    };

    constexpr auto foo = foo_t{};
    constexpr auto reflected_foo = refl::reflect_class(
        "foo_t",
        sizeof(foo_t),
        refl::refl_member::create<u32>("m_a", offsetof(foo_t, m_a)),
        refl::refl_member::create<u32>("m_b", offsetof(foo_t, m_b))
    );

    constexpr std::string_view class_name{ "foo_t" };
    STATIC_REQUIRE(class_name == reflected_foo.get_name());
    STATIC_REQUIRE(reflected_foo.get_member_count() == 2);
    STATIC_REQUIRE(reflected_foo.get_size_in_bytes() == sizeof(foo_t));

    constexpr auto reflected_foo_member_a = reflected_foo.get_member_at(0);
    STATIC_REQUIRE(reflected_foo_member_a.get_refl_type() == refl::refl_type::create<u32>());

    constexpr auto reflected_foo_by_macro = foo_t::get_reflected_class();
    STATIC_REQUIRE(reflected_foo == reflected_foo_by_macro);


    // TODO: fix
#if 0
    constexpr std::string_view expected_debug_string{ "foo_t{ }" };
    constexpr auto actual_debug_string = reflected_foo_by_macro.get_debug_string();
    constexpr std::string_view debug_string_view{ actual_debug_string };
    STATIC_REQUIRE(expected_debug_string == debug_string_view);
#endif

}

TEST_CASE("basic constexpr reflected class of wrapped type", "[meta::refl]")
{
    struct foo_t
    {
        std::vector<u32> m_a;

        KB_REFLECT_CLASS_START(foo_t)
        KB_REFLECT_MEMBER(m_a)
        KB_REFLECT_CLASS_END
    };

    foo_t foo{};
    constexpr auto reflected_foo = foo.get_reflected_class();

    // TODO: fix
#if 0
    constexpr auto reflected_foo_member = reflected_foo.get_member_at(0);
    STATIC_REQUIRE(reflected_foo_member.get_refl_type().get_type() == refl::type_t::vector);
    STATIC_REQUIRE(reflected_foo_member.get_refl_type().is_wrapped_type() == true);
    STATIC_REQUIRE(reflected_foo_member.get_refl_type().get_underlying_type() == refl::type_t::u32);
#endif
}

TEST_CASE("accessing reflected class member", "[meta::refl]")
{
    struct foo_t
    {
        u32 m_a;
        u32 m_b;

        KB_REFLECT_CLASS_START(foo_t)
            KB_REFLECT_MEMBER(m_a),
            KB_REFLECT_MEMBER(m_b)
            KB_REFLECT_CLASS_END
    };

    constexpr auto foo = foo_t{};
    constexpr auto reflected_foo = foo.get_reflected_class();

#if 0
    auto first_member = refl::get_member<>(foo, 0);
#endif
}
