#pragma once
#ifndef KABLUNK_CORE_EXPECTED_HPP
#define KABLUNK_CORE_EXPECTED_HPP

#include <type_traits>
#include <concepts>

// implementation of std::expected (from c++23) in c++20
// referenced implementation https://github.com/RishabhRD/expected/blob/master/include/rd/expected.hpp
// expected is based off the rust result<T, E> concept, which allows for error handling without c++'s runtime exception overhead

namespace kb
{ // start namespace Kablunk

// #TODO should probably be moved to concepts.hpp
namespace concepts
{ // start namespace ::concepts

// concept to check that the type is either void or destructible;
template <typename T>
concept non_void_destructible = std::same_as<T, void> || std::destructible<T>;

} // end namespace ::concepts

// forward declaration
// ensure that the value and error types are destructible
template <concepts::non_void_destructible T, std::destructible E>
class expected;

// #TODO should probably be moved to concepts.hpp
namespace concepts
{ // start namespace ::concepts

template <typename T>
concept is_expected = std::same_as<std::remove_cvref_t<T>, expected<typename T::value_t, typename T::error_t>>;

} // end namespace ::concepts

// expected implementation which takes a value and error type
// #TODO
template <concepts::non_void_destructible T, std::destructible E>
class expected
{
public:
	// typedef for underlying value type
	using value_t = T;
	// typedef for underlying error type
	using error_t = E;
public:
	// default constructor
	constexpr expected() requires std::default_initializable<value_t> 
		: m_value{} 
	{ }

	// copy constructor for expected value that is trivially constructible
	constexpr expected(const expected&) requires std::copy_constructible<value_t> && std::copy_constructible<error_t> &&
		std::is_trivially_copy_constructible_v<value_t> &&
		std::is_trivially_copy_constructible_v<error_t> = default;

	// copy constructor for expected value
	// ensures that after construction, this->has_value() == other.has_value()
	constexpr expected(const expected& other) requires std::copy_constructible<value_t> && std::copy_constructible<error_t>
		: m_has_value{ other.m_has_value }
	{ 
		// #TODO why use construct at instead of regular constructor?
		if (has_value())
			std::construct_at(std::addressof(m_value), *other);
		else
			std::construct_at(std::addressof(m_unexpected), other.error());
	}

	// move constructor for expected value that is trivially constructible
	// ensures that after construction, this->has_value() == other.has_value()
	constexpr expected(expected&&) noexcept(std::is_nothrow_move_constructible_v<value_t> && std::is_nothrow_move_constructible_v<value_t>) 
		requires std::move_constructible<value_t> && std::move_constructible<error_t> &&
		std::is_trivially_move_constructible_v<value_t> &&
		std::is_trivially_move_constructible_v<error_t> = default;

	// move constructor for expected value
	// ensures that after construction, this->has_value() == other.has_value()
	constexpr expected(expected&& other) noexcept(std::is_nothrow_move_constructible_v<value_t>&& std::is_nothrow_move_constructible_v<value_t>)
		requires std::move_constructible<value_t>&& std::move_constructible<error_t>
	: m_has_value{ other.m_has_value }
	{
		// #TODO why use construct at instead of regular constructor?
		if (has_value())
			std::construct_at(std::addressof(m_value), *other);
		else
			std::construct_at(std::addressof(m_unexpected), other.error());
	}

	// default destructor
	// ensures correct destructor is called
	constexpr ~expected()
	{
		if constexpr (std::is_trivially_destructible<value_t> && std::is_trivially_destructible<error_t>)
		{ 
			// do nothing since both possible values are trivially constructible, should be optimized away
		}
		else if constexpr (std::is_trivially_destructible<error_t> && !has_value())
			std::destroy_at(std::addressof(m_unexpected));
		else
		{
			if (has_value())
				std::destroy_at(std::addressof(m_value));
			else
				std::destroy_at(std::addressof(m_unexpected));
		}
	}

	// check whether this has a value
	[[nodiscard]] constexpr bool has_value() const noexcept { return m_has_value; }

	// =============================================
	// dereference and class member access operators
	// =============================================

	// class member access operator for value
	constexpr value_t* operator->() noexcept { return std::addressof(m_value); }
	// class member access operator for value
	constexpr const value_t* operator->() const noexcept { return std::addressof(m_value); }

	// dereference operator with l-value
	constexpr const value_t& operator*() const& noexcept { return m_value; }
	// dereference operator
	constexpr value_t& operator*() & noexcept { return m_value; }
	// dereference operator with r-value
	constexpr const value_t&& operator*() const&& noexcept { return std::move(m_value); }
	// dereference operator with r-value
	constexpr value_t&& operator*() && noexcept { return std::move(m_value); }

	// bool operator
	constexpr explicit operator bool() const noexcept { return m_has_value; }

	// ===============
	// value accessors
	// ===============

	// value accessor with const l-value
	constexpr const value_t& value() const&
	{
		if (has_value())
			return m_value;
		
		assert(false);
		// #TODO throw exception?
	}

	// value accessor with l-value
	constexpr value_t& value() &
	{
		if (has_value())
			return m_value;

		assert(false);
		// #TODO throw exception?
	}

	// value accessor with const r-value
	constexpr const value_t&& value() const&&
	{
		if (has_value())
			return std::move(m_value);

		assert(false);
		// #TODO throw exception?
	}

	// value accessor with r-value
	constexpr value_t&& value() &&
	{
		if (has_value())
			return std::move(m_value);

		assert(false);
		// #TODO throw exception?
	}

	// ===============
	// error accessors
	// ===============

	// error accessor with const l-value
	constexpr const error_t& error() const& { return m_unexpected; }
	// error accessor with l-value
	constexpr error_t& error() & { return m_unexpected; }
	// error accessor with const r-value
	constexpr const error_t&& error() const&& { return m_unexpected; }
	// error accessor with r-value
	constexpr error_t&& error() && { return m_unexpected; }

	// ==================
	// monadic operations
	// ==================

	// #TODO

	// ==================
	// equality operators
	// ==================

	// comparison between two expected values with same types
	constexpr bool operator==(const expected& other) const noexcept
	{
		// if one has a value and the other does not, not equal
		if (hash_value() != other.has_value())
			return false;

		return has_value() ? value() == other.value() : error() == other.error();
	}

	// comparison with an expected type with different value and error type
	template <typename T2, typename E2>
		// require that both value types and error types have equality comparison operators
	requires(!std::is_void_v(T2)) && 
		requires(const T& t1, const T2& t2, const error_t& e1, const E2& e2)
		{
			{ t1 == t2 } -> std::convertible_to<bool>;
			{ e1 == e2 } -> std::convertible_to<bool>;
		}
	constexpr bool operator==(const expected<T2, E2>& other) const
	{
		// if one has a value and the other does not, not equal
		if (has_value() != other.has_value())
			return false;

		return has_value() ? (value() == other.value()) : error() == other.error();
	}

	// comparison with an expected type with different value type
	template <typename T2>
	// require that error types have equality comparison operators
	requires(!concepts::is_expected<T2>) && requires(T const& t1, T2 const& t2)
	{
		{ t1 == t2 } -> std::convertible_to<bool>;
	}
	constexpr bool operator==(const expected<T2, error_t>& other) const
	{
		// compare error types
		return has_value() && static_cast<bool>(value() == other.value());
	}

	// comparison with an expected type with different error type
	template <typename E2>
	// require that error types have equality comparison operators
	requires requires(const E& e1, const E2& e2)
	{
		{ e1 == e2 } -> std::convertible_to<bool>;
	}
	constexpr bool operator==(const expected<value_t, E2>& other) const
	{
		// compare error types
		return !has_value() && static_cast<bool>(error() == other.error());
	}

private:
	// flag for whether this has a value or unexpected value
	bool m_has_value{ true };
	// union between value and unexpected type
	union
	{
		value_t m_value;
		error_t m_unexpected;
	};
};

} // end namespace Kablunk

#endif
