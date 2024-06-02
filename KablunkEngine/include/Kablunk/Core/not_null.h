#pragma once

#include "Kablunk/Core/KablunkAPI.h"

#include <type_traits>

namespace kb
{ // start namespace kb

// based on https://github.com/carbon-language/carbon-lang/blob/trunk/explorer/base/nonnull.h

// alias for a non-nullable pointer ex: not_null<T*> instead of T*
// sanitizers enforce this on assignment, return, and when passed as an argument
template <
    typename T,
    typename std::enable_if_t<std::is_pointer_v<T>>* = nullptr
>
using not_null_t = T KB_NOT_NULL;

} // end namespace kb
