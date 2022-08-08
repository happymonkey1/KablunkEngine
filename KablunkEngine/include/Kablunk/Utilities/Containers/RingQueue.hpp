#pragma once
#ifndef KABLUNK_UTILITIES_CONTAINER_RING_QUEUE_HPP
#define KABLUNK_UTILITIES_CONTAINER_RING_QUEUE_HPP

#include <Kablunk/Core/Logger.h>
#include <Kablunk/Core/CoreTypes.h>

namespace Kablunk
{

	template <typename T, u64 Size = 1024ull>
	class RingQueue
	{
	public:
		explicit RingQueue() noexcept;
		// Construct a RingQueue with data allocated elsewhere
		explicit RingQueue(void* data, size_t size, bool free_on_destroy) noexcept;
		RingQueue(const RingQueue&);
		RingQueue(RingQueue&&);
		~RingQueue() noexcept;

		// Free the stack (if managed by ring queue) and invalidate internal pointers
		void destroy();

		// Return a pointer to the head of the ring queue
		const T* peek_front() const { return m_data && m_read != m_write ? (m_data + m_read) : nullptr; };

		// Return a pointer to the head of the ring queue
		T* peek_front() const { return m_data && m_read != m_write ? (m_data + m_read) : nullptr; }

		// Pop the head from the ring queue
		void pop();

		// Copy insert data into the end of the ring queue
		void insert(const T& data);

		// Move insert data into the end of the ring queue
		void insert(T&& data);

		// Check if the ring queue is empty
		bool empty() const { return m_read == m_write; }

		// Get the number of elements in the ring queue
		u64 count() const { return m_count; }

		// Copy new ring queue
		RingQueue& operator=(const RingQueue&);

		// Move new ring queue
		RingQueue& operator=(RingQueue&&);

		// #TODO const iterator
	private:
		// pointer to the stack data
		T* m_data = nullptr;

		// index (pointer) to the head of the ring queue
		u64 m_read = 0;

		// index (pointer) to the tail of the ring queue
		u64 m_write = 0;

		// size of the container
		u64 m_length = Size;

		// number of elements in the container
		u64 m_count = 0;

		// flag for whether data should be freed on destruction
		bool m_free_on_destroy;
	};

	template <typename T, u64 Size /*= 1024ull*/>
	RingQueue<T, Size>::RingQueue() noexcept
		: m_data{ new T[Size] }, m_head{ 0 }, m_tail{ 0 }, m_length{ Size }, m_free_on_destroy{ true }
	{
		memset(m_data, 0, Size);

		KB_CORE_INFO("Created RingQueue of size={} bytes", sizeof(T) * Size)
	}

	template <typename T, u64 Size /*= 1024ull*/>
	RingQueue<T, Size>::RingQueue(void* data, size_t size, bool free_on_destroy) noexcept
		: m_data{ (T*)data }, m_head{ 0 }, m_tail{ 0 }, m_length{ size }, m_free_on_destroy{ free_on_destroy }
	{
		KB_CORE_ASSERT(Size == static_cast<u64>(size), "Allocation sizes do not match!");
		KB_CORE_ASSERT(data, "Data pointer is invalid!");
		
		// Most likely an error.
		if (free_on_destroy)
			KB_CORE_WARN("data separately allocated for ring queue but told to be freed on destruction...");

		memset(m_data, 0, size);
	}

	template <typename T, u64 Size /*= 1024ull*/>
	RingQueue<T, Size>::RingQueue(const RingQueue& other)
	{
		KB_CORE_ASSERT(other.m_data, "other ring queue data is not valid!");
		KB_CORE_ASSERT(m_length == other.m_length, "Stack sizes do not match!");

		m_data = new T[other.m_length];
		memset(m_data, 0, m_length);
		m_length = other.m_length;
		// Copy data. T must be copyable.
		for (size_t i = 0; i < m_length)
			*(m_data + i) = *(other.m_data + i);

		m_read = other.m_read;
		m_write = other.m_write;
		m_count = other.m_count;
		m_free_on_destroy = true;
	}

	template <typename T, u64 Size /*= 1024ull*/>
	RingQueue<T, Size>::RingQueue(RingQueue&& other)
	{
		KB_CORE_ASSERT(other.m_data, "other ring queue data is not valid!");
		KB_CORE_ASSERT(m_length == other.m_length, "Stack sizes do not match!");

		// swap
		m_data				= other.m_data;
		m_read				= other.m_read;
		m_write				= other.m_write;
		m_length			= other.m_length;
		m_count				= other.m_count;
		m_free_on_destroy	= other.m_free_on_destroy;

		// invalidate other ring queue
		other.m_data  = nullptr;
		other.m_read = 0;
		other.m_write = 0;
		other.m_count = 0;
	}


	template <typename T, u64 Size /*= 1024ull*/>
	RingQueue<T, Size>::~RingQueue() noexcept
	{
		destroy();
	}

	template <typename T, u64 Size /*= 1024ull*/>
	RingQueue<T, Size>& RingQueue<T, Size>::operator=(const RingQueue& other)
	{
		m_data = new T[other.m_length];
		memset(m_data, 0, m_length);
		m_length = other.m_length;
		// Copy data. T must be copyable.
		for (size_t i = 0; i < m_length)
			*(m_data + i) = *(other.m_data + i);

		m_read = other.m_read;
		m_write = other.m_write;
		m_count = other.m_count;
		m_free_on_destroy = true;

		return *this;
	}

	template <typename T, u64 Size /*= 1024ull*/>
	RingQueue<T, Size>& RingQueue<T, Size>::operator=(RingQueue&& other)
	{
		KB_CORE_ASSERT(other.m_data, "other ring queue data is not valid!");
		KB_CORE_ASSERT(m_length == other.m_length, "Stack sizes do not match!");

		// swap
		m_data = other.m_data;
		m_read = other.m_read;
		m_write = other.m_write;
		m_length = other.m_length;
		m_count = other.m_count;
		m_free_on_destroy = other.m_free_on_destroy;

		// invalidate other ring queue
		other.m_data = nullptr;
		other.m_read = 0;
		other.m_write = 0;
		other.m_count = 0;

		return *this;
	}

	template <typename T, u64 Size /*= 1024ull*/>
	void RingQueue<T, Size>::destroy()
	{
		if (m_data && m_free_on_destroy)
			delete[] m_data;

		m_data = nullptr;
		m_read = 0;
		m_write = 0;
	}

	template <typename T, u64 Size /*= 1024ull*/>
	void RingQueue<T, Size>::pop()
	{
		KB_CORE_ASSERT(m_data, "stack ptr is invalid!");
		m_read = (m_read + 1) % m_length;
		m_count--;
	}

	template <typename T, u64 Size /*= 1024ull*/>
	void RingQueue<T, Size>::insert(const T& data)
	{
		KB_CORE_ASSERT(m_data, "stack ptr is invalid!");
		*(m_data + m_write) = data;
		m_write = (m_write + 1) % m_length;
		m_count++;
	}

	template <typename T, u64 Size /*= 1024ull*/>
	void RingQueue<T, Size>::insert(T&& data)
	{
		KB_CORE_ASSERT(m_data, "stack ptr is invalid!");
		*(m_data + m_write) = data;
		m_write = (m_write + 1) % m_length;
		m_count++;
	}

}

#endif
