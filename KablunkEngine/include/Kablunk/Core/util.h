#pragma once

namespace kb
{ // start namespace kb

// reference: https://en.cppreference.com/w/cpp/utility/unreachable
[[noreturn]] inline void unreachable()
{
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
}

} // end namespace kb
