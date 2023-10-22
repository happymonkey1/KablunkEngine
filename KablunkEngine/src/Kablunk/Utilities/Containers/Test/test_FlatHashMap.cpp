#include "kablunkpch.h"
#include "Kablunk/Utilities/Containers/Test/test_FlatHashMap.h"

#include "Kablunk/Utilities/Containers/FlatHashMap.hpp"
#include "Kablunk/Core/ScopedTimer.h"

#include <stdint.h>
#include <string>
#include <unordered_map>

namespace kb::util::container::test
{ // start namespace kb::container::test

// main test function that runs unit tests
void test_flat_hash_map_main()
{
	test_flat_hash_map_constructors();
	test_flat_hash_map_insert_and_erase();
	test_flat_hash_map_iterators();
	test_flat_hash_map_find();
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

	// test hash map pair
	{
		details::hash_map_pair<std::string, uint32_t> p1{ "hello", 1 };
		details::hash_map_pair<std::string, uint32_t> p2{ "world", 2 };

		auto&& p3 = std::move(p1);

	}
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

	// clear the map
	{
		KB_CORE_TRACE("[Test]: flat_unordered_hash_map clear");
		test_map.clear();
		KB_CORE_ASSERT(test_map.size() == 0, "map is not empty after clear?");
		KB_CORE_ASSERT(test_map.max_size() == test_map.get_default_max_size(), "map rebuilt when it wasn't supposed to!");
		KB_CORE_TRACE("[Test]:   passed");
	}

	// test reserve and insert
	{
		KB_CORE_TRACE("[Test]: flat_unordered_hash_map reserve and insert 2048 values");
		test_map.reserve(4096);
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


	flat_unordered_hash_map<std::string, uint32_t> test_map{};


	// test iterator on empty map
	{
		KB_CORE_TRACE("[Test] [flat_unordered_hash_map]: forward iterator on default constructed map");
		size_t count = 0;
		for (auto& [key, value] : test_map)
		{
			KB_CORE_TRACE("  {} : {}", key, value);
			++count;
		}

		KB_CORE_ASSERT(test_map.size() == 0, "map should be empty!");
		KB_CORE_ASSERT(test_map.size() == count, "iterator returned an invalid pair!");
		KB_CORE_TRACE("[Test] [flat_unordered_hash_map]:   passed");
	}

	test_map.insert({ "hello", 1 });
	test_map.insert({ "world", 2 });
	test_map.insert({ "foo", 3 });
	test_map.insert({ "bar", 4 });

	KB_CORE_ASSERT(test_map.size() == 4, "insertion operator does not work!");

	// test iterator with four values
	{
		KB_CORE_TRACE("[Test] [flat_unordered_hash_map]: forward iterator on 4 values map");
		size_t count = 0;
		size_t sum = 0;
		for (auto& [key, value] : test_map)
		{
			KB_CORE_TRACE("  {} : {}", key, value);
			++count;
			sum += value;
		}

		KB_CORE_ASSERT(test_map.size() == count, "iterator count and map size do not match!");
		KB_CORE_ASSERT(sum == 10, "sum of values does not match!");
		KB_CORE_TRACE("[Test] [flat_unordered_hash_map]:   passed");
	}

	// test const iterator with four values

	// test iterator with 8192 values
	{
		KB_CORE_TRACE("[Test] [flat_unordered_hash_map]: forward iterator on 8192 values map");
		
		flat_unordered_hash_map<size_t, size_t> test_map_b;
		constexpr const size_t insert_count = 8192ull;
		for (size_t i = 0; i < insert_count; ++i)
			test_map_b.emplace({ i, i });

		size_t count = 0;
		size_t sum = 0;
		for (auto& [key, value] : test_map_b)
		{
			++count;
			sum += value;
		}

		KB_CORE_ASSERT(test_map_b.size() == insert_count, "insertion did not work correctly!");
		KB_CORE_ASSERT(count == insert_count, "iterator did not get every value!");
		KB_CORE_ASSERT(sum == 33550336, "sum of 0 ... 8191 does not match!");
		KB_CORE_TRACE("[Test] [flat_unordered_hash_map]:   passed");
	}
}

void test_flat_hash_map_find()
{
	// test iterator with four values
	{
		KB_CORE_TRACE("[Test] [flat_unordered_hash_map]: find()");
		flat_unordered_hash_map<std::string, uint64_t> test_map;

		auto key_to_find = std::string{ "hello" };
		test_map.insert({ key_to_find, 1 });
		test_map.insert({ "world", 2 });
		test_map.insert({ "foo", 3 });
		test_map.insert({ "bar", 4 });

		auto it_a = test_map.find(key_to_find);
		auto it_b = test_map.find("baz");
		KB_CORE_ASSERT(test_map.size() == 4, "map size does not contain 4 values, insertion must have failed!");
		KB_CORE_ASSERT(it_a != test_map.end(), "find() could not find '{}' in the map!", key_to_find);
		KB_CORE_ASSERT(it_a->key == key_to_find, "find() found '{}', but was supposed to return '{}'", it_a->key, key_to_find);
		KB_CORE_ASSERT(it_b == test_map.end(), "find() found '{}', but was not supposed to return anything!", it_b->key);
		KB_CORE_TRACE("[Test] [flat_unordered_hash_map]:   passed");
	}

	// test const iterator with four values

	// test iterator with 8192 values
	{
		KB_CORE_TRACE("[Test] [flat_unordered_hash_map]: forward iterator on 8192 values map");

		flat_unordered_hash_map<size_t, size_t> test_map_b;
		constexpr const size_t insert_count = 8192ull;
		for (size_t i = 0; i < insert_count; ++i)
			test_map_b.emplace({ i, i });

		size_t count = 0;
		size_t sum = 0;
		for (auto& [key, value] : test_map_b)
		{
			++count;
			sum += value;
		}

		KB_CORE_ASSERT(test_map_b.size() == insert_count, "insertion did not work correctly!");
		KB_CORE_ASSERT(count == insert_count, "iterator did not get every value!");
		KB_CORE_ASSERT(sum == 33550336, "sum of 0 ... 8191 does not match!");
		KB_CORE_TRACE("[Test] [flat_unordered_hash_map]:   passed");
	}
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
			test_map.insert({ std::to_string(i), i });
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
		for (size_t i = 0ull; i < insert_count; ++i)
			test_map.emplace({ std::to_string(i), i });
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
		for (size_t i = 0ull; i < insert_count; ++i)
			test_map.emplace(std::make_pair(std::to_string(i), i));
	}

	// test inserting 1048576 values into flat_unordered_hash_map
	{
		scoped_timer<> timer{
			[](std::chrono::nanoseconds nanos) -> void
			{
				KB_CORE_TRACE(
					"[Test] [flat_unordered_hash_map]: insert 1048576 values took {} ns",
					nanos.count()
				);
			}
		};
		flat_unordered_hash_map<std::string, uint64_t> test_map;
		constexpr const size_t insert_count = 1024ull * 1024ull;
		for (size_t i = 0ull; i < insert_count; ++i)
			test_map.emplace({ std::to_string(i), i });
	}

	// test inserting 1048576 values into std::unordered_map
	{
		scoped_timer<> timer{
			[](std::chrono::nanoseconds nanos) -> void
			{
				KB_CORE_TRACE(
					"[Test] [std::unordered_map]: insert 1048576 values took {} ns",
					nanos.count()
				);
			}
		};
		std::unordered_map<std::string, uint64_t> test_map;
		constexpr const size_t insert_count = 1024ull * 1024ull;
		for (size_t i = 0ull; i < insert_count; ++i)
			test_map.emplace(std::make_pair(std::to_string(i), i));
	}

	// test iteration through 1024 values
	{
		flat_unordered_hash_map<std::string, uint64_t> test_map_a;
		std::unordered_map<std::string, uint64_t> test_map_b;
		constexpr const size_t insert_count = 1024ull;
		for (size_t i = 0ull; i < insert_count; ++i)
		{
			test_map_a.insert({ std::to_string(i), i });
			test_map_b.insert({ std::to_string(i), i });
		}


		// test flat_unordered_hash_map
		{
			scoped_timer<> timer{
				[](std::chrono::nanoseconds nanos) -> void
				{
					KB_CORE_TRACE(
						"[Test] [flat_unordered_hash_map]: iterating 1024 values took {} ns",
						nanos.count()
					);
				}
			};

			size_t sum = 0ull;
			for (auto& [key, value] : test_map_a)
				sum += value;
			KB_CORE_INFO("[flat_unordered_hash_map]: sum of 1024 integers in a series: {}", sum);
		}

		// test std::unordered_map
		{
			scoped_timer<> timer{
				[](std::chrono::nanoseconds nanos) -> void
				{
					KB_CORE_TRACE(
						"[Test] [std::unordered_map]: iterating 1024 values took {} ns",
						nanos.count()
					);
				}
			};

			size_t sum = 0ull;
			for (auto& [key, value] : test_map_b)
				sum += value;
			KB_CORE_INFO("[std::unordered_map]: sum of 1024 integers in a series: {}", sum);
		}

	}

	// test iterating 8192 values
	{
		flat_unordered_hash_map<std::string, uint64_t> test_map_a;
		std::unordered_map<std::string, uint64_t> test_map_b;
		constexpr const size_t insert_count = 1024ull * 4ull;
		for (size_t i = 0ull; i < insert_count; ++i)
		{
			test_map_a.insert({ std::to_string(i), i });
			test_map_b.insert({ std::to_string(i), i });
		}


		// test flat_unordered_hash_map
		{
			scoped_timer<> timer{
				[](std::chrono::nanoseconds nanos) -> void
				{
					KB_CORE_TRACE(
						"[Test] [flat_unordered_hash_map]: iterating 8192 values took {} ns",
						nanos.count()
					);
				}
			};

			size_t sum = 0ull;
			for (auto& [key, value] : test_map_a)
				sum += value;
			KB_CORE_INFO("[flat_unordered_hash_map]: sum of 8192 integers in a series: {}", sum);
		}

		// test std::unordered_map
		{
			scoped_timer<> timer{
				[](std::chrono::nanoseconds nanos) -> void
				{
					KB_CORE_TRACE(
						"[Test] [std::unordered_map]: iterating 8192 values took {} ns",
						nanos.count()
					);
				}
			};

			size_t sum = 0ull;
			for (auto& [key, value] : test_map_b)
				sum += value;
			KB_CORE_INFO("[std::unordered_map]: sum of 8192 integers in a series: {}", sum);
		}

		KB_CORE_INFO("[Test] performance testing finished");
	}

}

} // end namespace kb::util::container::test
