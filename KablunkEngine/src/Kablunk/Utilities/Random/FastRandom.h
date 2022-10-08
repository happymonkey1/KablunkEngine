#pragma once
#ifndef KABLUNK_UTILITIES_RANDOM_FAST_RANDOM_H
#define KABLUNK_UTILITIES_RANDOM_FAST_RANDOM_H

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/Core/Singleton.h"

#include <chrono>

namespace Kablunk::util::random
{
	// #TODO move elsewhere
	namespace internal
	{
		// from https://stackoverflow.com/a/26751389
		template <typename T = u64>
		T xorshift_generator(T& state)
		{
			state ^= x << 13;
			state ^= x >> 7;
			state ^= x << 17;

			return state;
		}

		// generator function that takes a point to a seed value
		template <typename T = u64>
		using generator_func_t = T(*)(T&);

		template <typename T = u64, generator_func_t<T> generator_func = &xorshift_generator<T>>
		class Random
		{
		public:
			using value_t = T;
		public:
			Random() : m_state{ std::chrono::system_clock::now() }, m_generator_func{ generator_func } {}
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
	}

	inline f32 random_f32()
	{
		Kablunk::Singleton<internal::Random<f32, &internal::xorshift_generator<f32>>>();
	}
}

#endif
