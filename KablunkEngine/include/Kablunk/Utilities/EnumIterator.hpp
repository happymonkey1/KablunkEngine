#pragma once
#ifndef KABLUNK_UTILTIIES_ENUM_ITERATOR_HPP
#define KABLUNK_UTILTIIES_ENUM_ITERATOR_HPP

#include <type_traits>

namespace Kablunk::util
{
	// https://stackoverflow.com/a/31836401
	
	/*
	* To use, specialize the class like so
	* using foo_iterator = enum_iterator<foo, foo::one, foo::three>;
	* 
	* Then the specialized function can be iterated in a for loop
	* for (foo i : foo_iterator{})
	*	bar(i);
	*/
	template <typename T, T begin_val, T end_val>
	class enum_iterator
	{
	public:
		typedef typename std::underlying_type<T>::type value_t;
	public:
		enum_iterator() : m_value{ static_cast<value_t>(begin_val) } {}
		enum_iterator(const T& set_value) : m_value{ static_cast<value_t>(set_value) } {}

		// why is it a copy?
		enum_iterator operator++()
		{
			++m_value;
			return *this;
		}

		T operator*() { return static_cast<T>(m_value); }

		enum_iterator begin() { return *this; }
		enum_iterator end() 
		{ 
			// should this be static?
			static const enum_iterator end_iter = ++enum_iterator{ end_val };
			return end_iter;
		}

		bool operator==(const enum_iterator& other) { return m_value == other.m_value; }
		bool operator!=(const enum_iterator& other) { return !(*this == other); }
	private:
		u32 m_value;
	};
}

#endif
