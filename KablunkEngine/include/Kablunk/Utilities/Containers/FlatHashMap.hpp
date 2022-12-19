#pragma once
#ifndef KABLUNK_UTILITIES_CONTAINER_FLAT_HASH_MAP_HPP
#define KABLUNK_UTILITIES_CONTAINER_FLAT_HASH_MAP_HPP

#include <stdint.h>
#include <string>
#include <utility>
#include <type_traits>

#ifdef KB_API
#	include <Kablunk/Core/Core.h>
#else
#	error "flat_unordered_hash_map is only supported with the KablunkEngine right now"
#endif

namespace Kablunk::util::container
{ // start namespace Kablunk::util::container

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

	// #TODO research how to specialize void* template so arbitrary types can work
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

		static constexpr const flag_t occupied_bit_flag		= 0b01;
		static constexpr const flag_t end_of_map_bit_flag	= 0b10;

		// key that is used to hash and store the pair
		key_t key{};
		// value associated with key
		value_t value{};
		// bits that can be used as flag(s) for a hash map (default size of 8 bits)
		flag_t flags{};

		hash_map_pair() = default;
		~hash_map_pair() = default;
		hash_map_pair(const key_t& key, const value_t& value)
			: key{ key }, value{ value }, flags{}
		{ }
		hash_map_pair(const hash_map_pair& other)
			: key{ other.key }, value{ other.value }, flags{ other.flags }
		{ }
		hash_map_pair(hash_map_pair&& other)
			: key{ std::move(other.key) }, value{ std::move(other.value) }, flags{ other.flags }
		{ }

		// copy assign operator
		hash_map_pair& operator=(const hash_map_pair& other)
		{
			// call copy constructor and move rvalue
			*this = std::move(hash_map_pair{ other });
			return *this;
		}

		// move assign operator
		hash_map_pair& operator=(hash_map_pair&& other) noexcept
		{
			std::swap(key, other.key);
			std::swap(value, other.value);
			std::swap(flags, other.flags);

			return *this;
		}

		// check the lowest flag bit to see whether the slot is occupied
		bool is_slot_occupied() const { return (flags & hash_map_pair::occupied_bit_flag); }

		// overload for structured binding
		// returns reference
		template <std::size_t _index>
		std::tuple_element_t<_index, hash_map_pair>& get()
		{
			if constexpr (_index == 0) { return key; }
			if constexpr (_index == 1) { return value; }
		}

		// overload for structured binding
		// returns const reference
		template <std::size_t _index>
		const std::tuple_element_t<_index, hash_map_pair>& get() const
		{
			if constexpr (_index == 0) { return key; }
			if constexpr (_index == 1) { return value; }
		}

		// overload for structured binding
		// returns rvalue overload
		template <std::size_t _index>
		std::tuple_element_t<_index, hash_map_pair>&& get() const
		{
			if constexpr (_index == 0) { return std::move(key); }
			if constexpr (_index == 1) { return std::move(value); }
		}

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

	// iterator class for flat_unordered_hash_map
	// essentially it is a wrapper for a pointer to a pair in the map
	class iterator
	{
	public:
		// default constructor
		iterator() = default;
		// constructor that takes a hash map pair
		iterator(hash_map_pair_t* pair_ptr, hash_map_pair_t* end_of_map_ptr)
			: m_ptr{ pair_ptr }, m_end{ end_of_map_ptr }
		{ 
			// make sure we point to a valid pair
			find_next_valid_pair();
		}
		// copy constructor
		iterator(const iterator&) = default;
		// move constructor
		iterator(iterator&&) = default;
		// destructor
		~iterator() = default;

		// copy assignment operator
		iterator& operator=(const iterator& other)
		{
			m_ptr = other.m_ptr;
			m_end = other.m_end;
		}

		// dereferencing operator
		hash_map_pair_t& operator*() 
		{
			KB_CORE_ASSERT(m_ptr, "invalid pointer");

			return *m_ptr; 
		}
		
		// member access operator
		hash_map_pair_t* operator->() 
		{
			KB_CORE_ASSERT(m_ptr, "invalid pointer");

			return m_ptr; 
		}

		// equality comparison operator
		bool operator==(const iterator& other) const { return m_ptr == other.m_ptr; }
		// inequality comparison operator
		bool operator!=(const iterator& other) const { return !(*this == other); }

		// prefix increment operator
		iterator& operator++()
		{
			if (!m_ptr)
				return *this;

			find_next_valid_pair();

			return *this;
		}
	private:
		// increment the pointer so it points to a valid pair
		// sets to nullptr if it exceeds the end of the map or the original pointer is invalid
		void find_next_valid_pair()
		{
			// #TODO this should probably be an assertion
			if (!m_ptr)
				return;

			// increment pointer until it finds a valid slot, or is out of bounds
			while (++m_ptr < m_end && !m_ptr->is_slot_occupied())
				continue;

			// out of bounds check
			if (m_ptr >= m_end)
				m_ptr = nullptr;
		}
	private:
		// pointer to a slot in the hash map
		hash_map_pair_t* m_ptr = nullptr;
		// pointer to the end of the hash map
		// is there a way we can implement this iterator without an extra 8 bytes?
		hash_map_pair_t* m_end = nullptr;
	};


	// const iterator class for the map
	// operates the same as a normal iterator
	class citerator
	{
	public:
		// default constructor
		citerator() = default;
		// constructor that takes a hash map pair
		citerator(const hash_map_pair_t* pair_ptr, const hash_map_pair_t* end_of_map_ptr)
			: m_ptr{ pair_ptr }, m_end{ end_of_map_ptr }
		{ 
			if (m_ptr)
				find_next_valid_pair();
		}
		// copy constructor
		citerator(const citerator&) = default;
		// move constructor
		citerator(citerator&&) = default;
		// destructor
		~citerator() = default;

		// copy assignment operator
		citerator& operator=(const citerator& other)
		{
			m_ptr = other.m_ptr;
			m_end = other.m_end;
		}

		// dereferencing operator
		const hash_map_pair_t& operator*() const
		{
			KB_CORE_ASSERT(m_ptr, "invalid pointer");

			return *m_ptr;
		}

		// member access operator
		const hash_map_pair_t* operator->() const
		{
			KB_CORE_ASSERT(m_ptr, "invalid pointer");

			return m_ptr;
		}

		// equality comparison operator
		bool operator==(const citerator& other) const { return m_ptr == other.m_ptr; }
		// inequality comparison operator
		bool operator!=(const citerator& other) const { return !(*this == other); }

		// prefix increment operator
		citerator& operator++()
		{
			if (!m_ptr)
				return *this;

			find_next_valid_pair();

			return *this;
		}
	private:
		// increment the pointer so it points to a valid pair
		// sets to nullptr if it exceeds the end of the map or the original pointer is invalid
		void find_next_valid_pair()
		{
			// #TODO this should probably be an assertion
			if (!m_ptr)
				return;

			// increment pointer until it finds a valid slot, or is out of bounds
			while (++m_ptr < m_end && !m_ptr->is_slot_occupied())
				continue;

			// out of bounds check
			if (m_ptr >= m_end)
				m_ptr = nullptr;
		}
	private:
		// pointer to a slot in the hash map
		const hash_map_pair_t* m_ptr = nullptr;
		// pointer to the end of the hash map
		// is there a way we can implement this iterator without an extra 8 bytes?
		const hash_map_pair_t* m_end = nullptr;
	};

public:
	// default constructor
	flat_unordered_hash_map();
	// copy constructor
	flat_unordered_hash_map(const flat_unordered_hash_map& other);
	// move constructor
	flat_unordered_hash_map(flat_unordered_hash_map&& other);
	// destructor
	~flat_unordered_hash_map();

	// copy assign operator
	flat_unordered_hash_map& operator=(const flat_unordered_hash_map& other);
	// move assign operator
	flat_unordered_hash_map& operator=(flat_unordered_hash_map&& other);

	// ========
	// capacity
	// ========

	// check whether the map is empty
	inline bool empty() const { return m_element_count == 0; }
	// returns the number of key-value pairs in the map
	inline size_t size() const { return m_element_count; };
	// returns the maximum number of elements that can be in the map before re-allocation of underlying bucket(s)
	inline size_t max_size() const { return m_max_elements; };
	// return the default max element count of a map
	inline constexpr size_t get_default_max_size() { return s_default_max_elements; }

	// =========
	// modifiers
	// =========

	// clear all entries and free memory, map is now considered an invalid object unless it is re-initialized
	void destroy();
	// clear all the entries from the map and resize to default map size
	void clear();
	// clear all the entries from the map
	void clear_entries();
	// insert element into the map
	void insert(const hash_map_pair_t& pair);
	// insert an element into the map via key and pair
	void insert(const key_t& key, const value_t& value);
	// insert an element or assign if it already exists
	void insert_or_assign();
	// construct element in-place
	void emplace(hash_map_pair_t&& pair);
	// construct element in-place with a hint
	void emplace_hint();
	// insert in-place if the key does not exist, otherwise do nothing
	void try_emplace(hash_map_pair_t&& pair);
	// erase element(s) from the map
	void erase(const key_t& key);
	// swap the contents
	void swap(flat_unordered_hash_map& other);
	// extract nodes from the container, removing the pair from the map and copying to a new address
	// returns an owning pointer
	hash_map_pair_t* extract(const key_t& key);
	// splices nodes from another container
	void merge(const flat_unordered_hash_map& other);
	
	// splices nodes from another container
	// is there an optimization to be made where moving a container is faster?
	// void merge(flag_unordered_hash_map&& other);
	
	// reserve *more* memory for the map, throws an error if the operation tries to make the map smaller
	void reserve(size_t new_size);
	// resize the map to a specified size
	// can make the map smaller, but does not guarantee which keys will be kept
	void resize(size_t new_size);

	// ======
	// lookup
	// ======

	// access a specific element with bounds checking
	value_t& at(const key_t& key);
	// access a specific element with bounds checking
	const value_t& at(const key_t& key) const;
	// access or insert a specific element
	value_t& operator[](const key_t& key);
	// return the number of elements matching a certain key
	size_t count(const key_t& key) const;
	// finds the element with a certain key
	value_t& find(const key_t& key);
	// finds the element with a certain key
	const value_t& find(const key_t& key) const;
	// check if a key is contained within the map
	bool contains(const key_t& key) const;

	// =========
	// iterators
	// =========

	// iterator pointing to the beginning of the map
	iterator begin() { return iterator{ m_bucket, m_bucket + m_max_elements }; }
	// iterator pointing to the end of the map
	iterator end() { return iterator{ nullptr, (m_bucket + m_max_elements) }; }
	// const iterator pointing to the beginning of the map
	citerator cbegin() const { return citerator{ m_bucket, m_bucket + m_max_elements }; }
	// iterator pointing to the end of the map
	citerator cend() const { return citerator{ nullptr, (m_bucket + m_max_elements) }; }
private:
	// find the index of the bucket where a key lives if present
	inline size_t find_index_of(const key_t& key) const;
	// check if a bucket slot is occupied
	inline bool is_slot_occupied(const hash_map_pair_t& pair) const;
	// re-allocate a larger array, move old map's values, and free old map
	inline void rebuild();
private:
	// default size of map
	static constexpr const size_t s_default_max_elements = 1024ull;
	// count of elements in the map
	size_t m_element_count = 0ull;
	// maximum size of the bucket before re-allocation
	size_t m_max_elements = s_default_max_elements;
	// percentage the bucket can be filled before re-allocation
	float m_load_factor = 0.875f;
	// contiguous array of hash map pairs
	hash_map_pair_t* m_bucket = nullptr;
};

// ============================
// start implementation details
// ============================

// default constructor
template <typename K, typename V>
flat_unordered_hash_map<K, V>::flat_unordered_hash_map()
	: m_bucket{ new hash_map_pair_t[m_max_elements]{} }
{

}

// copy constructor for hash map with the same key and value type
template <typename K, typename V>
flat_unordered_hash_map<K, V>::flat_unordered_hash_map(const flat_unordered_hash_map& other)
{
	// reserve more space if needed
	if (other.max_size() > max_size())
		reserve(other.max_size());

	// do we need to go through every element and copy? could we just iterate through valid keys?
	// copy element wise
	for (size_t i = 0; i < m_max_elements; ++i)
		m_bucket[i] = other.m_bucket[i];
	
	m_max_elements = other.m_max_elements;
	m_element_count = other.m_element_count;
	m_load_factor = other.m_load_factor;
}

// move constructor for hash map with the same key and value type
template <typename K, typename V>
flat_unordered_hash_map<K, V>::flat_unordered_hash_map(flat_unordered_hash_map&& other)
{
	// swap contents with other map
	swap(other);
}

// destructor
template <typename K, typename V>
flat_unordered_hash_map<K, V>::~flat_unordered_hash_map()
{
	if (m_bucket)
		delete[] m_bucket;
#if KB_DEBUG
	// we should never have an invalid pointer
	else
		KB_CORE_ASSERT(false, "tried deleting invalid bucket pointer?");
#endif
}

// copy assign operator
template <typename K, typename V>
flat_unordered_hash_map<K, V>& flat_unordered_hash_map<K, V>::operator=(const flat_unordered_hash_map& other)
{
	*this = flat_unordered_hash_map<K, V>{ other };
}

// move assign operator
template <typename K, typename V>
flat_unordered_hash_map<K, V>& flat_unordered_hash_map<K, V>::operator=(flat_unordered_hash_map&& other)
{
	swap(other);
}

// free memory and invalid the map
template <typename K, typename V>
void flat_unordered_hash_map<K, V>::destroy()
{
	if (m_bucket)
		delete[] m_bucket;

	m_bucket = nullptr;
	m_element_count = 0;
}

// clear all the entries from the map
// frees current bucket, resizing to default size
template <typename K, typename V>
void flat_unordered_hash_map<K, V>::clear()
{
	// free memory
	if (m_bucket)
		delete[] m_bucket;

	// resize array to default size
	m_bucket = new hash_map_pair_t[s_default_max_elements]{};
	m_element_count = 0;
	m_max_elements = s_default_max_elements;
}

// clear all the entries from the map
template <typename K, typename V>
void flat_unordered_hash_map<K, V>::clear_entries()
{
	// null out memory and clear entry count
	if (m_bucket)
		for (size_t i = 0; i < m_max_elements; ++i)
			m_bucket[i] = hash_map_pair_t{};

	m_element_count = 0;
}

// insert element into the map. *safely* fails if the key is already present
template <typename K, typename V>
void flat_unordered_hash_map<K, V>::insert(const hash_map_pair_t& pair)
{
	KB_CORE_ASSERT(m_bucket, "bucket pointer is invalid, did you forget to construct the map?");

	// rebuild the map if we are getting too full
	if (m_element_count + 1 >= static_cast<uint64_t>(static_cast<float>(m_max_elements) * m_load_factor))
	{
#ifdef KB_DEBUG
		KB_CORE_INFO(
			"[flat_unordered_hash_map]: triggering rebuild, {} >= {}",
			m_element_count + 1,
			static_cast<uint64_t>(static_cast<float>(m_max_elements) * m_load_factor)
		);
#endif
		rebuild();
	}

	hash_map_pair_t& found_pair = m_bucket[find_index_of(pair.key)];
	if (is_slot_occupied(found_pair))
	{
#ifdef KB_DEBUG
		KB_CORE_ASSERT(false, "tried inserting but key was already present")
#else
		KB_CORE_WARN("[flat_unordered_hash_map]: tried inserting but key was already present!");
#endif
		return;
	}
	
	// #TODO should these values be moved? 
	found_pair.key = pair.key;
	found_pair.value = pair.value;
	found_pair.flags |= hash_map_pair_t::occupied_bit_flag;

	m_element_count++;
}

// insert an element into the map via key and pair
// constructs a hash map pair and calls the other insertion method
template <typename K, typename V>
void flat_unordered_hash_map<K, V>::insert(const K& key, const V& value)
{
	insert(hash_map_pair_t{ key, value });
}

template <typename K, typename V>
V& flat_unordered_hash_map<K, V>::find(const K& key)
{
	KB_CORE_ASSERT(m_bucket, "bucket pointer is invalid, did you forget to construct the map?");

	// #TODO this function should return an iterator so when the key is not present we can *safely* fail
	hash_map_pair_t& pair = m_bucket[find_index_of(key)];
	if (is_slot_occupied(pair))
		return pair.value;
	else
	{
		KB_CORE_ASSERT(false, "not implemented");
		return pair.value; 
	}
}

// find the index of the bucket where a key lives if present using open addressing. 
// this uses a naive implementation of linear probing open addressing from https://en.wikipedia.org/wiki/Open_addressing
template <typename K, typename V>
inline size_t flat_unordered_hash_map<K, V>::find_index_of(const K& key) const
{
	KB_CORE_ASSERT(m_bucket, "bucket pointer is invalid, did you forget to construct the map?");

	const hash_t hash_value = hash::generate_u64_fnv1a_hash(key);
	size_t index = hash_value % m_max_elements;
	// this can be subject to infinite loop if load balance == 1, though we should never get to that point...
	while (is_slot_occupied(m_bucket[index]) && m_bucket[index].key != key)
		index = (index + 1) % m_max_elements;

	return index;
}

// check whether a slot in the map is occupied
template <typename K, typename V>
inline bool flat_unordered_hash_map<K, V>::is_slot_occupied(const hash_map_pair_t& pair) const
{
	return pair.is_slot_occupied();
}

// rebuild the map, doubling its max element count
// moves old map entries to the new bucket
template <typename K, typename V>
inline void flat_unordered_hash_map<K, V>::rebuild()
{
	KB_CORE_ASSERT(m_bucket, "bucket pointer is invalid, did you forget to construct the map?");

	hash_map_pair_t* old_bucket = m_bucket;
	const size_t old_element_count = m_max_elements;

	// double size of new bucket
	m_max_elements *= 2;
	m_bucket = new hash_map_pair_t[m_max_elements]{};

	// copy old elements to new map
	for (size_t i = 0; i < old_element_count; ++i)
		if (is_slot_occupied(old_bucket[i]))
			m_bucket[find_index_of(old_bucket[i].key)] = std::move(old_bucket[i]);
		
	if (old_bucket)
		delete[] old_bucket;
}

// try inserting a value if the key does not exist in the map, otherwise assign the value at the key
template <typename K, typename V>
void flat_unordered_hash_map<K, V>::insert_or_assign()
{
	KB_CORE_ASSERT(false, "not implemented!");
}

// emplace a value in the map, does not care whether the key already exists or not
template <typename K, typename V>
void flat_unordered_hash_map<K, V>::emplace(hash_map_pair_t&& pair)
{
	KB_CORE_ASSERT(m_bucket, "bucket pointer is invalid, did you forget to construct the map?");

	hash_map_pair_t& found_pair = m_bucket[find_index_of(pair.key)];
	found_pair = std::move(pair);
}

template <typename K, typename V>
void flat_unordered_hash_map<K, V>::emplace_hint()
{
	KB_CORE_ASSERT(false, "not implemented!");
}

// try emplace a value in the map if the key does not exist, otherwise do nothing
template <typename K, typename V>
void flat_unordered_hash_map<K, V>::try_emplace(hash_map_pair_t&& pair)
{
	KB_CORE_ASSERT(m_bucket, "bucket pointer is invalid, did you forget to construct the map?");

	hash_map_pair_t& found_pair = m_bucket[find_index_of(pair.key)];
	if (is_slot_occupied(found_pair))
	{
#if KB_DEBUG
		KB_CORE_WARN("[flat_unordered_hash_map]: tried emplacing value but key '{}' already exists!", pair.key);
#endif
		return;
	}

	// is this correct? should we instead get a pointer to the pair and move with de-referencing?
	found_pair = std::move(pair);
}

// erase an entry from the map via key
template <typename K, typename V>
void flat_unordered_hash_map<K, V>::erase(const key_t& key)
{
	KB_CORE_ASSERT(m_bucket, "bucket pointer is invalid, did you forget to construct the map?");
	// should lazy deletion be implemented? this could be done by just setting the "occupied" bit of the pair

#ifdef KB_DEBUG
	KB_CORE_ASSERT(is_slot_occupied(m_bucket[find_index_of(key)]), "key does not exist in map!");
#endif

	// set memory at the index to zero, does not care whether the key actually exists in the map or not
	const size_t index = find_index_of(key);
	m_bucket[index] = hash_map_pair_t{};
	// std::memset(m_bucket + index, 0, sizeof(hash_map_pair_t));
}

template <typename K, typename V>
void flat_unordered_hash_map<K, V>::swap(flat_unordered_hash_map& other)
{
	// swap bucket pointers
	std::swap(m_bucket, other.m_bucket);
	// swap element count
	std::swap(m_element_count, other.m_element_count);
	// swap max load
	std::swap(m_max_elements, other.m_max_elements);
	// swap load factor
	std::swap(m_load_factor, other.m_load_factor);
}

// extract a pair from the map
// allocates new memory for the pair and returns an owning pointer
// zeros out the original entry in the map
template <typename K, typename V>
details::hash_map_pair<K, V>* flat_unordered_hash_map<K, V>::extract(const K& key)
{
	KB_CORE_ASSERT(m_bucket, "bucket pointer is invalid, did you forget to construct the map?");

	const size_t index = find_index_of(key);
	KB_CORE_ASSERT(is_slot_occupied(m_bucket[index]), "tried extracting a pair that does not exist in the map!");
	
	// copy pair to new memory address
	hash_map_pair_t* new_pair = new hash_map_pair_t;
	*new_pair = m_bucket[index];

	// zero out existing pair in map
	m_bucket[index] = hash_map_pair_t{};
	// std::memset(m_bucket[index], 0, sizeof(hash_map_pair_t));

	return new_pair;
}

// merge (mutation) two maps together
template <typename K, typename V>
void flat_unordered_hash_map<K, V>::merge(const flat_unordered_hash_map& other)
{
	KB_CORE_ASSERT(m_bucket, "bucket pointer is invalid, did you forget to construct the map?");

	// #TODO should we just reserve a size big enough in one pass?
	while (size() + other.size() >= static_cast<size_t>(static_cast<float>(m_max_elements) * m_load_factor))
		rebuild();

	// iterate through other map and insert values
	for (size_t i = 0; i < other.m_max_elements; ++i)
		if (is_slot_occupied(other.m_bucket[i]))
			insert(other.m_bucket[i]);
}

// reserve more space in the map
// throws error if the operation attempts to make the map smaller
// size is number of elements (not size in bytes)
template <typename K, typename V>
void flat_unordered_hash_map<K, V>::reserve(size_t new_size)
{
	KB_CORE_ASSERT(m_bucket, "bucket pointer is invalid, did you forget to construct the map?");

	KB_CORE_ASSERT(m_max_elements < new_size, "cannot resize map to be smaller!")

	// move old bucket to newly allocated space
	hash_map_pair_t* old_bucket = m_bucket;
	m_bucket = new hash_map_pair_t[new_size]{};
	std::memmove(m_bucket, old_bucket, sizeof(hash_map_pair_t) * m_max_elements);
	m_max_elements = new_size;

	if (old_bucket)
		delete[] old_bucket;
}

// resize the map to a specific size
// can make the map smaller, but will not guarantee which keys remain
template <typename K, typename V>
void flat_unordered_hash_map<K, V>::resize(size_t new_size)
{
	KB_CORE_ASSERT(m_bucket, "bucket pointer is invalid, did you forget to construct the map?");
	KB_CORE_ASSERT(new_size > 0, "cannot resize map to size 0, try using clear() instead");

	hash_map_pair_t* old_bucket = m_bucket;

	m_bucket = new hash_map_pair_t[new_size]{};

	// insert old elements into new map
	const size_t min_new_size = std::min(m_max_elements, new_size);
	for (size_t i = 0; i < min_new_size; ++i)
		if (m_bucket[i].is_slot_occupied())
			insert(m_bucket);

	if (old_bucket)
		delete[] old_bucket;
}

// returns a reference to a value via key
// exception occurs if the key does not exist
template <typename K, typename V>
V& flat_unordered_hash_map<K, V>::at(const K& key)
{
	const size_t index = find_index_of(key);
	hash_map_pair_t& found_pair = m_bucket[index];

	KB_CORE_ASSERT(is_slot_occupied(found_pair), "key does not exist in the map!");

	return found_pair;
}

// returns a reference to a value via key
// exception occurs if the key does not exist
template <typename K, typename V>
const V& flat_unordered_hash_map<K, V>::at(const K& key) const
{
	const size_t index = find_index_of(key);
	hash_map_pair_t& found_pair = m_bucket[index];

	KB_CORE_ASSERT(is_slot_occupied(found_pair), "key does not exist in the map!");

	return found_pair;
}

// index operator
template <typename K, typename V>
V& flat_unordered_hash_map<K, V>::operator[](const K& key)
{
	return m_bucket[find_index_of(key)].value;
}

// counting the number of key entries in the map does not make sense since we only use one bucket?
template <typename K, typename V>
size_t flat_unordered_hash_map<K, V>::count(const K& key) const
{
	KB_CORE_ASSERT(false, "not implemented");
	return 0;
}

// check whether the map contains a specific key
template <typename K, typename V>
bool flat_unordered_hash_map<K, V>::contains(const K& key) const
{
	return is_slot_occupied(m_bucket[find_index_of(key)]);
}

// ==========================
// end implementation details
// ==========================

} // end namespace Kablunk::util::container

// overload for structured binding
namespace std
{ // start namespace std

// let the tuple now how many elements it contains
template<typename K, typename V>
struct tuple_size<Kablunk::util::container::details::hash_map_pair<K, V>>
	: integral_constant<size_t, 2> {};

// tuple element 0 specialization
template<typename K, typename V>
struct tuple_element<0, Kablunk::util::container::details::hash_map_pair<K, V>>
{
	using type = K;
};

// tuple element 1 specialization
template<typename K, typename V>
struct tuple_element<1, Kablunk::util::container::details::hash_map_pair<K, V>>
{
	using type = V;
};

} // end namespace std

#endif
