#pragma once
#ifndef KABLUNK_UTILITY_ALGORITHM_STRING_SEARCHING_H
#define KABLUNK_UTILITY_ALGORITHM_STRING_SEARCHING_H

#include <vector>
#include <string>
#include <string_view>

namespace Kablunk::util::algorithm::search
{ // start namespace Kablunk::util::algorithm::search

namespace details
{ // start namespace ::details

	// #TODO this should be moved to a utility hash file

	// static constexpr "magic" values for the rolling hash function
	struct magic_values
	{
		// constant whose inverse is multiplied against each byte in the input key
		static constexpr const size_t a{ 69420 };
		// random prime number that which is modulo-ed (is that a word?) against all math operations in the rolling hash function
		static constexpr const size_t n{ 73018877 };
	};

	// compute a polynomial rolling hash
	// implementation details from https://en.wikipedia.org/wiki/Rolling_hash
	// since the rabin-karp string matching function needs to compute multiple hashes
	// over a rolling window, we need a fast hash function that avalanches from window to window
	// the algorithm is as follows:
	//     H = c1 * a^(k-1) + c2 * a^(k-2) + ... + ck * a^0
	//     where c1, ..., ck are input characters, and a is a constant
	// this implementation uses division and addition, and all math is done modulo n to avoid huge hashes
	// the choice of *a* and *n* is critical to getting a good hash
	size_t compute_polynomial_rolling_hash(const std::string_view& key)
	{
		size_t hash = 0;
		// multiple each byte in the string by a^(-i)
		for (size_t i = 0; i < key.size(); ++i)
		{
			// #NOTE(sean) should we care about decimals...?
			const size_t byte_hash = (static_cast<size_t>(key[i]) / (static_cast<size_t>(std::pow(magic_values::a, i)) % magic_values::n)) % magic_values::n;
			// make sure we can never overflow
			hash = (hash + byte_hash) % magic_values::n;
		}
		
		return hash;
	}

	// removing the first character simply involves subtracting the first term of the polynomial
	size_t remove_first_character(const size_t hash, const size_t character_index, const std::string_view& key)
	{
		const size_t key_byte_as_u64 = static_cast<size_t>(key[character_index]);
		const size_t term_constant = (static_cast<size_t>(std::pow(magic_values::a, character_index)) % magic_values::n);
		return (hash - ((key_byte_as_u64 / term_constant) % magic_values::n)) % magic_values::n;
	}

} // end namespace ::details

// rabin-karp string matching algorithm
// implemented using https://en.wikipedia.org/wiki/Rabin%E2%80%93Karp_algorithm
std::vector<std::string> rabin_karp_string_match(const std::string_view& search_data, const std::string_view& pattern);

} // end namespace Kablunk::util::algorithm::search

#endif
