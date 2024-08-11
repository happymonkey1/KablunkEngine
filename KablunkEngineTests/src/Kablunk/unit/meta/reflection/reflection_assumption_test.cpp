#include "Kablunk/meta/reflection/reflect.h"

#include <catch_amalgamated.hpp>

using namespace kb;
using namespace kb::meta;

template <typename T>
consteval auto get_type_id_hash() -> decltype(auto)
{
    return typeid(T).hash_code();
}

TEST_CASE("assumption", "[meta::refl]")
{
    // Does not result in constexpr value
    // constexpr auto foo = get_type_id_hash<u32>();

    constexpr auto foo2 = COMPILE_TIME_CRC32_STR(refl::details::type_name<u32>::value.data());
}
