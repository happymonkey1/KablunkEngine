#pragma once

namespace kb::concepts
{ // start namespace kb::concepts

template <typename T>
concept TrivialT = std::is_trivial_v<T>;

template <typename T>
concept NumericT = std::integral<T> || std::floating_point<T>;

} // end namespace kb::concepts
