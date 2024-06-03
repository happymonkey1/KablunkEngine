#pragma once

// #TODO move to meta folder and move under kb::meta::concepts namespace
namespace kb::concepts
{ // start namespace kb::concepts

template <typename T>
concept TrivialT = std::is_trivial_v<T>;

template <typename T>
concept NumericT = std::integral<T> || std::floating_point<T>;

template<typename T>
concept EnumT = std::is_enum_v<T>;

} // end namespace kb::concepts
