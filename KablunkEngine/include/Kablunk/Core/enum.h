#pragma once

#include "Kablunk/Core/concepts.hpp"

namespace kb
{ // start namespace kb

// Helper that returns the casted value of an enum
template <concepts::EnumT T>
constexpr auto to_underlying(T p_value) noexcept -> std::underlying_type_t<T>
{
    return static_cast<std::underlying_type_t<T>>(p_value);
}

} // end namespace kb
