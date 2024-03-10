#pragma once

#include <cstdint>
#include <string_view>
#include <fmt/format.h>

namespace kb
{ // start namespace kb

namespace details
{ // start namespace ::details

template <typename T, size_t Size = 128>
struct fixed_big_int
{
    using value_t = T;

    value_t m_data[Size / (sizeof(T) * 8)]{ 0 };

    auto operator<=>(const fixed_big_int&) const = default;
};

} // end namespace ::details

using u128 = details::fixed_big_int<uint8_t>;
using i128 = details::fixed_big_int<int8_t>;

} // end namespace kb
