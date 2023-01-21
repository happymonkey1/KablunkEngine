#pragma once

#ifndef KABUNK_CORE_MEMORY_LINEAR_ALLOCATOR_H
#define KABUNK_CORE_MEMORY_LINEAR_ALLOCATOR_H

#include "Kablunk/Core/KablunkAPI.h"
#include "Kablunk/Core/CoreTypes.h"

#include <memory>

namespace Kablunk::memory
{

	/* 
	Linear allocator which just increments a pointer for every allocated object
	Default size of 10mb
	*/
	template <size_t Size = 102400Ui64>
	class LinearAllocator
	{
	public:
		LinearAllocator()
			: m_block{ new u8[Size] }, m_head{ m_block }
		{ }

		~LinearAllocator() { reset(); }

		template <typename T>
		[[nodiscard]] T* allocate(size_t size)
		{
			// check that there is still room in the block
			if (m_head + size + sizeof(memory::LinearBlockHeader) >= m_size)
				throw std::bad_array_new_length();

			// capture pointer for object
			T* ptr = static_cast<T*>(m_head);

			// increment head by the size of the object
			m_head += size;

			return ptr;
		}

		template <typename T>
		[[nodiscard]] void deallocate(T* ptr, size_t size, bool alloc = true) noexcept
		{
			// does nothing
		}

		void reset() noexcept
		{
			if (m_block)
				delete[] m_block;

			m_block = nullptr;
			m_head = nullptr;
			m_size = 0xDEADBEEF;
		}
	private:
		// pointer to the allocated block
		u8* m_block = nullptr;
		// pointer to the head of the block
		u8* m_head = nullptr;
		// size of the block in bytes
		size_t m_size = Size;
	};

}

#endif
