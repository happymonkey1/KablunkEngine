#pragma once
#ifndef KABLUNK_UTILITIES_CONTAINER_FLAT_HASH_MAP_HPP
#define KABLUNK_UTILITIES_CONTAINER_FLAT_HASH_MAP_HPP

#include <stdint.h>
#include <string>

#ifdef KB_PLATFORM_WINDOWS
#	include <Kablunk/Core/Core.h>
#else
#	error "flat_unordered_hash_map is only supported with the KablunkEngine right now"
#endif

namespace Kablunk::Utilities
{ // start namespace Kablunk::Utilities

namespace hash
{ // start namespace ::hash

	// algorithm from https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
	template <typename T> 
	inline uint64_t generate_u64_fnv1a_hash(const T& value)
	{
		static_assert(false, "value_t does not support hashing!");
	}

	// template specialization for uint64_t
	template <>
	inline uint64_t generate_u64_fnv1a_hash(const uint64_t& value)
	{
		static constexpr const uint64_t FNV_offset_basis = 14695981039346656037ull;
		// FNV prime is large prime
		static constexpr const uint64_t FNV_prime = 1099511628211ull;
		uint64_t hashed_value = FNV_offset_basis;

		// iterate through each byte of data to hash
		for (size_t i = 0; i < sizeof(value); ++i)
		{
			// xor the lower 8 bits of the hash with the current byte of data
			hashed_value = ((hashed_value >> 8) & 0xFF) ^ ((value >> (i * 8)) & 0xFF);
			// multiply hashed value with prime constant
			hashed_value = hashed_value * FNV_prime;
		}

		return hashed_value;
	}

	// template specialization for void*
	/*template <>
	inline uint64_t generate_u64_fnv1a_hash(const void*& value)
	{
		// cast to uint64_t and use that specialization
		return generate_u64_fnv1a_hash<uint64_t>(static_cast<uint64_t>(value));
	}*/

	template <>
	inline uint64_t generate_u64_fnv1a_hash<std::string>(const std::string& value)
	{
		static constexpr const uint64_t FNV_offset_basis = 14695981039346656037ull;
		// FNV prime is large prime
		static constexpr const uint64_t FNV_prime = 1099511628211ull;
		uint64_t hashed_value = FNV_offset_basis;

		// iterate through each byte of data to hash
		for (size_t i = 0; i < value.size(); ++i)
		{
			// xor the lower 8 bits of the hash with the current byte of data
			hashed_value = (hashed_value & 0xFF) ^ static_cast<uint8_t>(value[i]);
			// multiply hashed value with prime constant
			hashed_value = hashed_value * FNV_prime;
		}

		return hashed_value;
	}

} // end namespace ::hash

namespace details
{ // start namespace ::details
	template <typename K, typename V, typename flag_type = uint8_t>
	struct hash_map_pair
	{
		using key_t = K;
		using value_t = V;
		using flag_t = flag_type;

		// key that is used to hash and store the pair
		key_t key{};
		// value associated with key
		value_t value{};
		// bits that can be used as flag(s) for a hash map (default size of 8 bits)
		flag_t flags{};

		hash_map_pair() = default;
		~hash_map_pair() = default;
		hash_map_pair(const hash_map_pair& other)
			: key{ other.key }, value{ other.value }, flags{ other.flags }
		{ }
		hash_map_pair(hash_map_pair&& other)
			: key{ std::move(other.key) }, value{ std::move(other.value) }, flags{ other.flags }
		{ }
	};
} // end namespace ::details

template <typename K, typename V>
class flat_unordered_hash_map
{
public:
	using key_t = K;
	using value_t = V;
	using hash_map_pair_t = details::hash_map_pair<key_t, value_t>;
	using hash_t = uint64_t;
public:
	flat_unordered_hash_map();
	~flat_unordered_hash_map();

	// ========
	// capacity
	// ========

	// check whether the map is empty
	inline bool empty() const { return m_element_count > 0; }
	// returns the number of key-value pairs in the map
	inline size_t size() const { return m_element_count; };
	// returns the maximum number of elements that can be in the map before re-allocation of underlying bucket(s)
	inline size_t max_size() const { return m_max_elements; };

	// =========
	// modifiers
	// =========

	// clear all the entries from the map
	void clear();
	// clear all the entries from the map and deallocate array
	void clear_and_deallocate();
	// insert element into the map
	void insert(const hash_map_pair_t& pair);
	// insert an element or assign if it already exists
	void insert_or_assign();
	// construct element in-place
	void emplace();
	// construct element in-place with a hint
	void emplace_hint();
	// insert in-place if the key does not exist, otherwise do nothing
	void try_emplace();
	// erase element(s) from the map
	void erase();
	// swap the contents
	void swap();
	// extract nodes from the container
	void extract();
	// splices nodes from another container
	void merge();

	// ======
	// lookup
	// ======

	// access a specific element with bounds checking
	value_t& at();
	// access a specific element with bounds checking
	const value_t& at() const;
	// access or insert a specific element
	value_t& operator[](const key_t& key);
	// return the number of elements matching a certain key
	size_t count() const;
	// finds the element with a certain key
	value_t& find(const key_t& key);
	// finds the element with a certain key
	const value_t& find(const key_t& key) const;
	// check if a key is contained within the map
	bool contains() const;
private:
	// find the index of the bucket where a key lives if present
	inline size_t find_index_of(const key_t& key) const;
	// check if a bucket slot is occupied
	inline bool is_slot_occupied(const hash_map_pair_t& pair) const;
	// re-allocate a larger array, move old map's values, and free old map
	inline void rebuild();
private:
	// contiguous array of hashmap pairs
	hash_map_pair_t* m_bucket = nullptr;
	// count of elements in the map
	size_t m_element_count = 0;
	// maximum size of the bucket before re-allocation
	size_t m_max_elements = 1024;
	// percentage the bucket can be filled before re-allocation
	float m_load_factor = 0.875f;
};

// ============================
// start implementation details
// ============================

// default constructor
template <typename K, typename V>
flat_unordered_hash_map<K, V>::flat_unordered_hash_map()
{
	m_bucket = new hash_map_pair_t[m_max_elements];
}

// destructor
template <typename K, typename V>
flat_unordered_hash_map<K, V>::~flat_unordered_hash_map()
{
	if (m_bucket)
		delete[] m_bucket;
}

// clear all the entries from the map
template <typename K, typename V>
void flat_unordered_hash_map<K, V>::clear()
{
	// null out memory and clear entry count
	if (m_bucket)
		std::memset(m_bucket, 0, sizeof(hash_map_pair_t) * m_max_elements);
	m_element_count = 0;
}

// clear all the entries from the map and deallocate array
template <typename K, typename V>
void flat_unordered_hash_map<K, V>::clear_and_deallocate()
{
	// free memory and clear entry count
	if (m_bucket)
		delete[] m_bucket;
	m_element_count = 0;
}

// insert element into the map. *safely* fails if the key is already present
template <typename K, typename V>
void flat_unordered_hash_map<K, V>::insert(const hash_map_pair_t& pair)
{
	hash_map_pair_t& found_pair = m_bucket[find_index_of(pair.key)];
	if (is_slot_occupied(found_pair))
	{
#ifdef KB_DEBUG
		KB_CORE_WARN("[flat_unordered_hash_map]: tried inserting '{}' but key was already present!", key);
#endif
		return;
	}
	
	// rebuild the map if we are getting too full
	if (m_element_count + 1 >= static_cast<uint64_t>(static_cast<float>(m_max_elements) * m_load_factor))
		rebuild();

	// #TODO should these values be moved? 
	found_pair.key = pair.key;
	found_pair.value = pair.value;
	found_pair.flags |= 0b1;

	m_element_count++;
}

template <typename K, typename V>
V& flat_unordered_hash_map<K, V>::find(const K& key)
{
	// #TODO this function should return an iterator so when the key is not present we can *safely* fail
	hash_map_pair_t& pair = m_bucket[find_index_of(key)];
	if (is_slot_occupied(pair))
		return pair.value;
	else
		KB_CORE_ASSERT(false, "iterator not implemented");
}

// find the index of the bucket where a key lives if present using open addressing. 
// this uses a naive implementation of linear probing open addressing from https://en.wikipedia.org/wiki/Open_addressing
template <typename K, typename V>
inline size_t flat_unordered_hash_map<K, V>::find_index_of(const key_t& key) const
{
	hash_t hash_value = hash::generate_u64_fnv1a_hash(key);
	size_t index = hash_value % m_max_elements;
	// this can be subject to infinite loop if load balance == 1
	while (is_slot_occupied(m_bucket[index]) && m_bucket[index].key != key)
		index = (index + 1) % m_max_elements;

	return index;
}

template <typename K, typename V>
inline bool flat_unordered_hash_map<K, V>::is_slot_occupied(const hash_map_pair_t& pair) const
{
	// check the lowest bit of flag to see if this space is occupied
	return (pair.flags & 0b1) == 0b1;
}

template <typename K, typename V>
inline void flat_unordered_hash_map<K, V>::rebuild()
{
	hash_map_pair_t* old_map_ptr = m_bucket;
	size_t old_element_count = m_max_elements;

	m_max_elements *= 2;
	m_bucket = new hash_map_pair_t[m_max_elements];

	for (size_t i = 0; i < m_max_elements; ++i)
	{
		if (!is_slot_occupied(old_map_ptr[i]))
			continue;

		hash_map_pair_t& old_pair = old_map_ptr[i];
		m_bucket[find_index_of(old_pair.key)] = std::move(old_pair);
	}

	delete[] old_map_ptr;
}

// ==========================
// end implementation details
// ==========================

} // end namespace Kablunk::Utilities

#endif
