#include "kablunkpch.h"

#include "Kablunk/Core/Memory/Memory.h"

#include "Kablunk/Core/Singleton.h"

#include "Kablunk/Core/Logger.h"

namespace kb::memory
{
	GeneralAllocator::GeneralAllocator()
		: m_data{ nullptr }, m_is_initializing{ false }
	{

	}

	const GeneralAllocator& GeneralAllocator::get()
	{
		return Singleton<GeneralAllocator>::get();
	}

	const size_t GeneralAllocator::get_total_allocated()
	{
		return get().m_data->global_allocation_stats.total_allocated;
	}

	const size_t GeneralAllocator::get_total_freed()
	{
		return get().m_data->global_allocation_stats.total_freed;
	}

	const GeneralAllocator::allocation_stats_map_t& GeneralAllocator::get_allocation_statistics_map()
	{
		return get().m_data->allocation_statistics_map;
	}

	void GeneralAllocator::init()
	{
		if (m_data)
		{
			KB_CORE_WARN("GeneralAllocator memory tracking already initialized!");
			return;
		}

		m_is_initializing = true;
		{
			general_allocator_data_t* data = static_cast<general_allocator_data_t*>(allocate_raw(sizeof(general_allocator_data_t)));
			// call constructor using previously allocated memory
			new(data) general_allocator_data_t{};
			m_data = data;
		}
		m_is_initializing = false;

		KB_CORE_INFO("Memory tracking initialized.");
	}

	void GeneralAllocator::shutdown()
	{
		delete m_data;
		m_data = nullptr;

		m_is_initializing = false;
	}

	void* GeneralAllocator::allocate_raw(size_t size)
	{
		return std::malloc(size);
	}

	void* GeneralAllocator::allocate(size_t size)
	{
		if (m_is_initializing)
			return allocate_raw(size);

		if (!m_data)
			init();

		void* mem_block = allocate_raw(size);
		{
			std::scoped_lock<std::mutex> lock{ m_data->mutex };
			map_allocation_block_t& alloc_block = m_data->allocation_map[mem_block];
			alloc_block.memory = mem_block;
			alloc_block.size = size;

			m_data->global_allocation_stats.total_allocated += size;
		}

		return mem_block;
	}

	void* GeneralAllocator::allocate(size_t size, const char* desc)
	{
		if (m_is_initializing)
			return allocate_raw(size);

		if (!m_data)
			init();

		void* mem_block = allocate_raw(size);
		{
			std::scoped_lock<std::mutex> lock{ m_data->mutex };
			map_allocation_block_t& alloc_block = m_data->allocation_map[mem_block];
			alloc_block.memory = mem_block;
			alloc_block.size = size;
			alloc_block.category = desc;

			m_data->global_allocation_stats.total_allocated += size;
			if (desc)
				m_data->allocation_statistics_map[alloc_block.category].total_allocated += size;
		}

		return mem_block;
	}

	void* GeneralAllocator::allocate(size_t size, const char* file, int line)
	{
		if (m_is_initializing)
			return allocate_raw(size);

		if (!m_data)
			init();

		void* mem_block = allocate_raw(size);
		{
			std::scoped_lock<std::mutex> lock{ m_data->mutex };
			map_allocation_block_t& alloc_block = m_data->allocation_map[mem_block];
			alloc_block.memory = mem_block;
			alloc_block.size = size;
			alloc_block.category = file;

			m_data->global_allocation_stats.total_allocated += size;
			if (file)
				m_data->allocation_statistics_map[alloc_block.category].total_allocated += size;
		}

		return mem_block;
	}

	void GeneralAllocator::free(void* memory)
	{
		if (!memory)
			return;

		{
			std::scoped_lock<std::mutex> lock{ m_data->mutex };
			if (m_data->allocation_map.find(memory) != m_data->allocation_map.end())
			{
				const map_allocation_block_t& alloc_block = m_data->allocation_map.at(memory);
				m_data->global_allocation_stats.total_freed += alloc_block.size;
				if (alloc_block.category)
					m_data->allocation_statistics_map[alloc_block.category].total_freed += alloc_block.size;

				m_data->allocation_map.erase(memory);
			}
			else
				KB_CORE_ERROR("memory block not found in allocation map!");
		}

		std::free(memory);
	}

	GeneralAllocator& GeneralAllocator::get_mutable()
	{
		return Singleton<GeneralAllocator>::get();
	}

}

/*
#ifdef KB_TRACK_MEMORY

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new(size_t size)
{
	return kb::memory::GeneralAllocator::get_mutable().allocate(size);
}

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new[](size_t size)
{
	return kb::memory::GeneralAllocator::get_mutable().allocate(size);
}

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new(size_t size, const char* desc)
{
	return kb::memory::GeneralAllocator::get_mutable().allocate(size, desc);
}

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new[](size_t size, const char* desc)
{
	return kb::memory::GeneralAllocator::get_mutable().allocate(size, desc);
}

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new(size_t size, const char* file, int line)
{
	return kb::memory::GeneralAllocator::get_mutable().allocate(size, file, line);
}

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new[](size_t size, const char* file, int line)
{
	return kb::memory::GeneralAllocator::get_mutable().allocate(size, file, line);
}

void __CRTDECL operator delete(void* memory)
{
	return kb::memory::GeneralAllocator::get_mutable().free(memory);
}

void __CRTDECL operator delete(void* memory, const char* desc)
{
	return kb::memory::GeneralAllocator::get_mutable().free(memory);
}

void __CRTDECL operator delete(void* memory, const char* file, int line)
{
	return kb::memory::GeneralAllocator::get_mutable().free(memory);
}

void __CRTDECL operator delete[](void* memory)
{
	return kb::memory::GeneralAllocator::get_mutable().free(memory);
}

void __CRTDECL operator delete[](void* memory, const char* desc)
{
	return kb::memory::GeneralAllocator::get_mutable().free(memory);
}

void __CRTDECL operator delete[](void* memory, const char* file, int line)
{
	return kb::memory::GeneralAllocator::get_mutable().free(memory);
}

#endif
*/
