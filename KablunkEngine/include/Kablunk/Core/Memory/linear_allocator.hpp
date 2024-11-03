#pragma once
#ifndef KABUNK_CORE_MEMORY_LINEAR_ALLOCATOR_H
#define KABUNK_CORE_MEMORY_LINEAR_ALLOCATOR_H

#include "Kablunk/Core/KablunkAPI.h"
#include "Kablunk/Core/CoreTypes.h"

#include <memory>

namespace kb::memory
{

/** 
* Linear allocator which just increments a pointer for every allocated object
* Default size of 10mb
*/
template <size_t Size = 102400Ui64>
class linear_allocator
{
public:
	linear_allocator()
		: m_block{ new u8[Size] }, m_head{ m_block }
	{ }

	~linear_allocator() { reset(); }

    linear_allocator(const linear_allocator&) noexcept = delete;
    auto operator=(const linear_allocator&) noexcept -> linear_allocator& = delete;

    linear_allocator(linear_allocator&& p_other) noexcept
        : m_block{ p_other.m_block }, m_head{ p_other.m_head }
    {}

    auto operator=(linear_allocator&& p_other) noexcept -> linear_allocator&
    {
        m_block = p_other.m_block;
        m_head = p_other.m_head;

        p_other.m_block = nullptr;
        p_other.m_head = nullptr;

        return *this;
    }

	template <typename T>
	[[nodiscard]] auto allocate() noexcept -> T*
	{
        constexpr size_t k_size = sizeof(T);

		// check that there is still room in the block
        KB_CORE_ASSERT(m_head + k_size < m_head + Size, "[linear_allocator]: OOM!");

		// capture pointer for object
		T* ptr = static_cast<T*>(m_head);

		// increment head by the size of the object
		m_head += k_size;

		return ptr;
	}

	template <typename T>
	auto deallocate() noexcept -> void
	{
		// no-op
	}

	void reset() noexcept
	{
		delete[] m_block;

		m_block = nullptr;
		m_head = nullptr;
	}

private:
	// pointer to the allocated block
	u8* m_block = nullptr;
	// pointer to the head of the block
	u8* m_head = nullptr;
};

}

#endif
