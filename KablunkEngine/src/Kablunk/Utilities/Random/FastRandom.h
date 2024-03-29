#pragma once
#ifndef KABLUNK_UTILITIES_RANDOM_FAST_RANDOM_H
#define KABLUNK_UTILITIES_RANDOM_FAST_RANDOM_H

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/Core/Singleton.h"

#include <chrono>

namespace kb::util::random
{
	// #TODO move elsewhere
	// from https://stackoverflow.com/a/26751389
	template <typename T>
	T xorshift_generator(T& state)
	{
		u64 state_u64 = static_cast<u64>(state);
		state_u64 ^= state_u64 << 13;
		state_u64 ^= state_u64 >> 7;
		state_u64 ^= state_u64 << 17;

		state = static_cast<T>(state_u64);

		return state;
	}

	// generator function that takes a point to a seed value
	template <typename T = u64>
	using generator_func_t = T(*)(T&);

	template <typename T = u64, generator_func_t<T> generator_func = &xorshift_generator<T>>
	class Random
	{
	public:
		Random() : m_state{ static_cast<T>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) }, m_generator_func{generator_func} {}
		~Random() = default;

		void set_seed(T seed) { m_state = seed; }
		T get_seed() const { return m_state; }

		T operator()()
		{
			return m_generator_func(m_state);
		}
	private:
		// function used to generate random values
		generator_func_t<T> m_generator_func;
		// initial seed
		T m_state;
	};
	

	inline f32 random_f32()
	{
        kb::Singleton<Random<f32, &xorshift_generator<f32>>>::get()();
	}
}

#endif
