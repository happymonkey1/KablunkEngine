#include "kablunkpch.h"
#include "Kablunk/Utilities/Containers/Test/test_FlatHashMap.h"

#include "Kablunk/Utilities/Containers/FlatHashMap.hpp"
#include "Kablunk/Core/ScopedTimer.h"

#include <stdint.h>
#include <string>
#include <unordered_map>

namespace Kablunk::util::container::test
{ // start namespace Kablunk::util::container::test

// main test function that runs unit tests
void test_flat_hash_map_main()
{
	test_flat_hash_map_constructors();
	test_flat_hash_map_insert_and_erase();
	test_flat_hash_map_iterators();
	test_flat_hash_map_performance();
}

// test constructors and assign operators
void test_flat_hash_map_constructors()
{
	// ========================
	// test default constructor
	// ========================

	flat_unordered_hash_map<std::string, uint32_t> test_map{};

	KB_CORE_ASSERT(test_map.size() == 0, "invalid size for default constructor");
	KB_CORE_ASSERT(test_map.empty(), "map should be empty!");

	// ========================
}

// test insertion and removal operators
void test_flat_hash_map_insert_and_erase()
{
	flat_unordered_hash_map<std::string, uint32_t> test_map{};

	KB_CORE_ASSERT(test_map.size() == 0, "default constructed map is not empty?");

	// try insert 256 value into the map
	// this should not trigger rebuild();
	{
		KB_CORE_TRACE("[Test]: flat_unordered_hash_map insert 256 values");
		const size_t insert_count = 256;
		for (size_t i = 0; i < insert_count; ++i)
			test_map.insert(std::to_string(i), i);

		KB_CORE_ASSERT(test_map.size() == insert_count, "insertion failed?");
		KB_CORE_ASSERT(test_map.max_size() == test_map.get_default_max_size(), "map rebuilt when it wasn't supposed to!");
		KB_CORE_TRACE("[Test]:   passed");
	}

	// test clearing the map
	{
		KB_CORE_TRACE("[Test]: flat_unordered_hash_map clear");
		test_map.clear();
		KB_CORE_ASSERT(test_map.size() == 0, "map is not empty after clear?");
		KB_CORE_ASSERT(test_map.max_size() == test_map.get_default_max_size(), "map rebuilt when it wasn't supposed to!");
		KB_CORE_TRACE("[Test]:   passed");
	}

	// test inserting 1024 values into the map
	// this should trigger rebuild once
	{
		KB_CORE_TRACE("[Test]: flat_unordered_hash_map insert 1024 values");
		const size_t insert_count = 1024;
		for (size_t i = 0; i < insert_count; ++i)
			test_map.insert(std::to_string(i), i);

		KB_CORE_ASSERT(test_map.size() == insert_count, "insertion failed?");
		KB_CORE_ASSERT(test_map.max_size() == test_map.get_default_max_size() * 2, "map rebuilt more than it was supposed to!");
		KB_CORE_TRACE("[Test]:   passed");
	}

	// clear the map
	{
		KB_CORE_TRACE("[Test]: flat_unordered_hash_map clear");
		test_map.clear();
		KB_CORE_ASSERT(test_map.size() == 0, "map is not empty after clear?");
		KB_CORE_ASSERT(test_map.max_size() == test_map.get_default_max_size(), "map rebuilt when it wasn't supposed to!");
		KB_CORE_TRACE("[Test]:   passed");
	}

	// test inserting 2048 values into the map
	// this should trigger rebuild twice
	{
		KB_CORE_TRACE("[Test]: flat_unordered_hash_map insert 2048 values");
		const size_t insert_count = 2048;
		for (size_t i = 0; i < insert_count; ++i)
			test_map.insert(std::to_string(i), i);

		KB_CORE_ASSERT(test_map.size() == insert_count, "insertion failed?");
		KB_CORE_ASSERT(test_map.max_size() == test_map.get_default_max_size() * 4, "map rebuilt when it wasn't supposed to!");
		KB_CORE_TRACE("[Test]:   passed");
	}
}

// test iterators
void test_flat_hash_map_iterators()
{

	KB_CORE_TRACE("test forward iterator");

	flat_unordered_hash_map<std::string, uint32_t> test_map{};


	// test iterator on empty map
	{
		size_t count = 0;
		for (auto& [key, value] : test_map)
		{
			KB_CORE_TRACE("  {} : {}", key, value);
			++count;
		}

		KB_CORE_ASSERT(test_map.size() == 0, "map should be empty!");
		KB_CORE_ASSERT(test_map.size() == count, "iterator returned an invalid pair!");
	}

	test_map.insert("hello", 1);
	test_map.insert("world", 2);
	test_map.insert({ "foo", 3 });
	test_map.insert({ "bar", 4 });

	KB_CORE_ASSERT(test_map.size() == 4, "insertion operator does not work!");

	// test iterator with four values
	{
		size_t count = 0;
		for (auto& [key, value] : test_map)
		{
			KB_CORE_TRACE("  {} : {}", key, value);
			++count;
		}

		KB_CORE_ASSERT(test_map.size() == count, "iterator count and map size do not match!");
	}

	// test const iterator with four values
}

// test performance compared to std::unordered_map
void test_flat_hash_map_performance()
{

	// test default flat_unordered_hash_map constructor
	{
		scoped_timer<> timer{ 
			[](std::chrono::nanoseconds nanos) -> void 
			{ 
				KB_CORE_TRACE(
					"[Test] [flat_unordered_hash_map]: default constructor took {} ns", 
					nanos.count()
				); 
			} 
		};
		flat_unordered_hash_map<std::string, uint64_t> test_map;
	}

	// test default std::unordered_map constructor
	{
		scoped_timer<> timer{ [](std::chrono::nanoseconds nanos) -> void { KB_CORE_TRACE("[Test] [unordered_map]: default constructor took {} ns", nanos.count()); } };
		std::unordered_map<std::string, uint64_t> test_map;
	}

	// test inserting 1024 values into flat_unordered_hash_map
	{
		scoped_timer<> timer{
			[](std::chrono::nanoseconds nanos) -> void
			{
				KB_CORE_TRACE(
					"[Test] [flat_unordered_hash_map]: insert 1024 values took {} ns",
					nanos.count()
				);
			}
		};
		flat_unordered_hash_map<std::string, uint64_t> test_map;
		constexpr const size_t insert_count = 1024ull;
		for (size_t i = 0ull; i < insert_count; ++i)
			test_map.insert(std::to_string(i), i);
	}

	// test inserting 1024 values into std::unordered_map
	{
		scoped_timer<> timer{
			[](std::chrono::nanoseconds nanos) -> void
			{
				KB_CORE_TRACE(
					"[Test] [std::unordered_map]: insert 1024 values took {} ns",
					nanos.count()
				);
			}
		};
		std::unordered_map<std::string, uint64_t> test_map;
		constexpr const size_t insert_count = 1024ull;
		for (size_t i = 0ull; i < insert_count; ++i)
			test_map.insert({ std::to_string(i), i });
	}

	// test inserting 131072 values into flat_unordered_hash_map
	{
		scoped_timer<> timer{
			[](std::chrono::nanoseconds nanos) -> void
			{
				KB_CORE_TRACE(
					"[Test] [flat_unordered_hash_map]: insert 131072 values took {} ns",
					nanos.count()
				);
			}
		};
		flat_unordered_hash_map<std::string, uint64_t> test_map;
		constexpr const size_t insert_count = 1024ull * 128ull;
		test_map.reserve(insert_count * 2);
		for (size_t i = 0ull; i < insert_count; ++i)
			test_map.insert(std::to_string(i), i);
	}

	// test inserting 131072 values into std::unordered_map
	{
		scoped_timer<> timer{
			[](std::chrono::nanoseconds nanos) -> void
			{
				KB_CORE_TRACE(
					"[Test] [std::unordered_map]: insert 131072 values took {} ns",
					nanos.count()
				);
			}
		};
		std::unordered_map<std::string, uint64_t> test_map;
		constexpr const size_t insert_count = 1024ull * 128ull;
		test_map.reserve(insert_count * 2);
		for (size_t i = 0ull; i < insert_count; ++i)
			test_map.insert({ std::to_string(i), i });
	}

}

} // end namespace Kablunk::util::container::test
