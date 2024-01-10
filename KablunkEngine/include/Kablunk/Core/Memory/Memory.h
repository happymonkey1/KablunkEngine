#pragma once
#ifndef KABLUNK_CORE_MEMORY_H
#define KABLUNK_CORE_MEMORY_H

#include "Kablunk/Core/KablunkAPI.h"

#include <memory>
#include <mutex>
#include <map>
#include <stdint.h>

namespace kb::memory
{ // start namespace kb::memory

// struct that hold total allocation statistics for a specific category
struct allocation_stats_t
{
	// total allocated memory for the mapped category
	size_t total_allocated = 0;
	// total freed memory for the mapped category
	size_t total_freed = 0;
};

// struct that holds a header to an block of map allocated memory
struct map_allocation_block_t
{
	// pointer to allocated memory
	void* memory = nullptr;
	// size of allocated memory block
	size_t size = 0;
	// category of memory for KablunkEngine statistics reporting
	const char* category = nullptr;
};

template <typename T>
struct map_allocator_t
{
	using value_type = T;
	map_allocator_t() = default;
	template <class U> constexpr map_allocator_t(const map_allocator_t<U>&) noexcept {}

	T* allocate(std::size_t n)
	{
#undef max
		if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
			throw std::bad_array_new_length();

		if (auto p = static_cast<T*>(std::malloc(n * sizeof(T)))) 
			return p;

		throw std::bad_alloc();
		return (T*)static_cast<u64>(0xDEADBEEF);
	}

	void deallocate(T* p, std::size_t n) noexcept {
		std::free(p);
	}
};

// #TODO refactor singleton to use some sort of dependency injection so we can overload new operators
class GeneralAllocator
{
public:
	GeneralAllocator();
	// type def for the allocation stats
	using allocation_stats_map_t = std::map<const char*, allocation_stats_t, std::less<const char*>>;
	// type def for map allocator
	using allocation_map_t = std::map<
		const void*, 
		map_allocation_block_t, 
		std::less<const void*>, 
		map_allocator_t<std::pair<const void* const, map_allocation_block_t>>
		>;
public:
	// =============
	// Singleton API
	// =============
		
	// get singleton pointing to the general allocator
	static const GeneralAllocator& get();
	// get mutable singleton pointer to the general allocator
	static GeneralAllocator& get_mutable();
	// get total allocated memory in bytes
	static const size_t get_total_allocated();
	// get total freed memory in bytes
	static const size_t get_total_freed();
	// get allocation statistics map
	static const allocation_stats_map_t& get_allocation_statistics_map();

	// ============
	// Internal API
	// ============

	// initialize the general allocator
	void init();
	// shutdown the general allocator
	void shutdown();
	// allocate memory without tracking
	void* allocate_raw(size_t size);
	// allocate memory with tracking
	void* allocate(size_t size);
	// allocate memory with descriptor and tracking
	void* allocate(size_t size, const char* desc);
	// allocate memory for file with tracking
	void* allocate(size_t size, const char* file, int line);
	// free memory with tracking
	void free(void* memory);
private:
	// struct to hold all the general allocator's data so we can allocate data without tracking itself, running into an initialization recursion error...
	struct general_allocator_data_t
	{
		// maps categories to allocation headers
		allocation_map_t allocation_map;
		// maps categories to their allocation statistics
		allocation_stats_map_t allocation_statistics_map;
		// global allocation statistics struct
		allocation_stats_t global_allocation_stats;
		// mutex for general allocator
		std::mutex mutex;
		// mutex for the allocation statistics
		std::mutex statistics_mutex;
	};
private:
	general_allocator_data_t* m_data = nullptr;
	// flag showing whether we are in initialization of the general allocator, meaning we should not track any memory allocations
	bool m_is_initializing = false;
};

} /* end kb::memory */

// /* Global namespace */

#ifdef KB_TRACK_MEMORY

/*
	_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
	void* __CRTDECL operator new(size_t size);

	_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
	void* __CRTDECL operator new[](size_t size);

	_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
	void* __CRTDECL operator new(size_t size, const char* desc);

	_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
	void* __CRTDECL operator new[](size_t size, const char* desc);

	_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
	void* __CRTDECL operator new(size_t size, const char* file, int line);

	_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
	void* __CRTDECL operator new[](size_t size, const char* file, int line);

	void __CRTDECL operator delete(void* memory);
	void __CRTDECL operator delete(void* memory, const char* desc);
	void __CRTDECL operator delete(void* memory, const char* file, int line);
	void __CRTDECL operator delete[](void* memory);
	void __CRTDECL operator delete[](void* memory, const char* desc);
	void __CRTDECL operator delete[](void* memory, const char* file, int line);
*/

#	define kb_new(T)				static_cast<decltype(T)*>(::kb::memory::GeneralAllocator::get_mutable().allocate(sizeof(T), __FILE__, __LINE__))
#   define kb_new_array(T, count)	static_cast<T*>(::kb::memory::GeneralAllocator::get_mutable().allocate(sizeof(T) * count, __FILE__, __LINE__))
#	define kb_delete(T)				::kb::memory::GeneralAllocator::get_mutable().free((void*)T)
#	define kb_delete_array(T)		::kb::memory::GeneralAllocator::get_mutable().free((void*)T);
#else
#	define kb_new(T)				new
#	define kb_new_array(T, count)	new T[count]
#	define kb_delete(T)				delete T
#	define kb_delete_array(T)		delete[] T
#endif

#endif
