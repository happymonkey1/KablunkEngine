#pragma once
#ifndef KABLUNK_CORE_BUFFER_H
#define KABLUNK_CORE_BUFFER_H

#include "Kablunk/Core/Core.h"

namespace kb
{
	class Buffer : public RefCounted
	{
	public:
		Buffer() : m_data{ nullptr }, m_size{ 0 } {}
		Buffer(void* data, size_t size) : m_data{ (u8*)data }, m_size{ size } {}
		Buffer(size_t size) : m_data{ nullptr }, m_size{ size } { Allocate(size); }
		Buffer(const Buffer& other) : m_data{ nullptr }, m_size{ other.m_size }
		{
			if (other.m_data)
			{
				Allocate(m_size);
				if (m_data && other.m_data)
					memcpy(m_data, other.m_data, m_size);
				else
					KB_CORE_ASSERT(false, "memcpy C6381 warning");
			}
		}

		Buffer(Buffer&& other) noexcept
			: m_data{ other.m_data }, m_size{ other.m_size }
		{
			other.m_size = 0;
			other.m_data = nullptr;
		}


		~Buffer()
		{
			Release();
		}

		inline Buffer& operator=(const Buffer& other)
		{
			if (other.m_data != nullptr)
			{
				m_size = other.m_size;
				Allocate(m_size);
				if (m_data && other.m_data)
					memcpy(m_data, other.m_data, m_size);
				else
					KB_CORE_ASSERT(false, "memcpy C6381 warning");
			}

			return *this;
		}

		Buffer& operator=(Buffer&& other) noexcept
		{
			m_size = other.m_size, other.m_size = 0;
			m_data = other.m_data, other.m_data = nullptr;
			return *this;
		}

		inline static Buffer Copy(const void* data, size_t size)
		{
			Buffer buffer;
			buffer.Allocate(size);

            KB_CORE_ASSERT(buffer.m_data, "[Buffer] Copy(): destination buffer is null?");

			memcpy(buffer.m_data, data, size);
			return buffer;
		}

		// size in bytes
		inline void Allocate(size_t size)
		{
			if (m_data)
			{
				delete[] m_data;
				m_data = nullptr;
			}

			if (size == 0)
            {
                KB_CORE_WARN("[Buffer]: allocating a buffer of size 0!");
				return;
            }

			//m_data = new uint8_t[size];
			m_data = kb_new_array(u8, size);
			m_size = size;
		}

		inline void Release()
		{
			kb_delete(m_data);
			// detete[] m_data;
			m_data = nullptr;
			m_size = 0;
		}

		inline void ZeroInitialize()
		{
			if (m_data && m_size > 0)
				memset(m_data, 0, m_size);
			else
				KB_CORE_ERROR("Trying to Zero Initialize an invalid buffer!");
		}

		template <typename T>
		T& Read(uint32_t offset)
		{
			KB_CORE_ASSERT(offset + sizeof(T) <= m_size, "trying to access memory out of bounds!");
			return *(T*)(m_data + offset);
		}

		inline uint8_t* ReadBytes(size_t size, size_t offset)
		{
			KB_CORE_ASSERT(offset + size <= m_size, "Buffer overflow!");
			u8* buffer = kb_new_array(u8, size);
			memcpy(buffer, m_data + offset, size);
			return buffer;
		}

		inline void Write(void* data, size_t size, size_t offset = 0)
		{
			KB_CORE_ASSERT(offset + size <= m_size, "Buffer overflow!");
			memcpy(m_data + offset, data, size);
		}

		operator bool() const { return m_data; }

		u8& operator[](size_t index) { return m_data[index]; }

		u8 operator[](size_t index) const { return m_data[index]; }

		template <typename T>
		T* As() const { return (T*)m_data; }

		inline size_t size() const { return m_size; }
		inline void* get() { return (void*)m_data; }
		inline const void* get() const { return (void*)m_data; }
	private:
		// capacity of the buffer
		size_t m_size;
		// pointer to the head of the block of memory
		u8* m_data;
	};
}

#endif
