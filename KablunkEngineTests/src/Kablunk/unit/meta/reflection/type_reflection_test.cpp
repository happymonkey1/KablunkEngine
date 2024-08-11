#include "Kablunk/meta/reflection/type.h"


#include <catch_amalgamated.hpp>

#include "Kablunk/meta/reflection/reflect.h"

using namespace kb;



TEST_CASE("basic constexpr relfected type", "[meta::refl]")
{
    using type_to_reflect_t = u32;
    constexpr meta::refl::refl_type reflected_type = meta::refl::refl_type::create<type_to_reflect_t>();

    STATIC_REQUIRE(reflected_type.get_type() == meta::refl::type_t::u32);
    STATIC_REQUIRE(KB_REFL_TYPE_SIZE(reflected_type) == sizeof(type_to_reflect_t));

    struct foo_t{};

    constexpr meta::refl::refl_type reflected_class = meta::refl::refl_type::create<foo_t>();
    STATIC_REQUIRE(reflected_class.get_type() == meta::refl::type_t::clazz);

    // TODO: fix
#if 0
    STATIC_REQUIRE(KB_REFL_TYPE_SIZE(reflected_class) == sizeof(foo_t));
#endif
}

TEST_CASE("basic constexpr type with constexpr reflected type", "[meta::refl]")
{
    constexpr u32 constexpr_foo = 0ull;
    constexpr auto reflected_foo = meta::refl::refl_type::create<decltype(constexpr_foo)>();
    STATIC_REQUIRE(reflected_foo.get_type() == meta::refl::type_t::u32);
    STATIC_REQUIRE(reflected_foo.is_const() == true);
    STATIC_REQUIRE(reflected_foo.is_pointer() == false);
}

TEST_CASE("basic constexpr reflected type casting", "[meta::refl]")
{
    u32 foo = 1000;
    constexpr auto foo_reflected = meta::refl::refl_type::create<decltype(foo)>();

    STATIC_REQUIRE(foo_reflected.get_type() == meta::refl::type_t::u32);

    auto foo_casted = KB_REFL_CONSTEXPR_CAST(foo_reflected, &foo);
    REQUIRE(foo == foo_casted);

    auto* foo_ptr = &foo;
    constexpr auto foo_ptr_reflected = meta::refl::refl_type::create<decltype(foo_ptr)>();
    STATIC_REQUIRE(foo_ptr_reflected.get_type() == meta::refl::type_t::u32);
    STATIC_REQUIRE(foo_ptr_reflected.is_pointer() == true);
    auto* foo_pointer_casted = KB_REFL_CONSTEXPR_CAST(foo_ptr_reflected, foo_ptr);
    REQUIRE(foo == foo_casted);
}
