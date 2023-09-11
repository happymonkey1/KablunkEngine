#pragma once
#ifndef KABLUNK_CORE_UNIQUE_HPP
#define KABLUNK_CORE_UNIQUE_HPP

#include <type_traits>
#include <concepts>

namespace kb
{ // start namespace Kablunk

// #TODO should probably be moved to concepts.hpp
namespace memory::concepts
{ // start namespace ::concepts

// concept to check that the type is either void or destructible;
template <typename T>
concept non_void_destructible = std::same_as<T, void> || std::destructible<T>;

} // end namespace ::concepts

// the unique<T> state is a unique ownership of T, with no outstanding references to this object, and can be passed around
// #TODO allocator
template <memory::concepts::non_void_destructible T>
class unique
{
public:
	// typedef for underlying value type
	using value_t = T;
private:
	// trivially constructible default constructor
	constexpr unique() requires std::is_default_constructible<value_t>
		: m_data{ new m_data{} }
	{ }

	// move constructor
	constexpr unique(T&& p_data) requires std::is_move_constructible_v<value_t>
		: m_data{ new m_data{ std::move(p_data) } }
	{ }

	// move constructor with variadic template arguments
	template <typename... args_t>
	constexpr unique(args_t&&... args) // #TODO requires for variadic template args
		: m_data{ new m_data{ std::forward<args_t>(args)...} }
	{ }
private:
	// pointer to the uniquely owned data (heap allocated)
	T* m_data = nullptr;
};


} // end namespace Kablunk

#endif
