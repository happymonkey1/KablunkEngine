#pragma once
#ifndef KABLUNK_UTILITIES_TEST_TEST_FLAT_HASH_MAP_H
#define KABLUNK_UTILITIES_TEST_TEST_FLAT_HASH_MAP_H

namespace Kablunk::util::container::test
{  // start namespace Kablunk::util::container::test

// main test function that runs unit tests
void test_flat_hash_map_main();
// test constructors and assign operators
void test_flat_hash_map_constructors();
// test insertion and removal operators
void test_flat_hash_map_insert_and_erase();
// test iterators
void test_flat_hash_map_iterators();
// test find
void test_flat_hash_map_find();
// test performance compared to std::unordered_map
void test_flat_hash_map_performance();

}  // end namespace Kablunk::util::container::test

#endif
