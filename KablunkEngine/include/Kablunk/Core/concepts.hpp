

namespace kb::concepts
{ // start namespace kb::concepts

template <typename T>
concept TrivialT = std::is_trivial_v<T>;

} // end namespace kb::concepts
