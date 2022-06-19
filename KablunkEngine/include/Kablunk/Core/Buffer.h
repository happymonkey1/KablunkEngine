#pragma once
#ifndef KABLUNK_CORE_BUFFER_H
#define KABLUNK_CORE_BUFFER_H

#include "Kablunk/Core/Core.h"

namespace Kablunk
{
	class Buffer : public RefCounted
	{
	public:
		Buffer() : m_data{ nullptr }, m_size{ 0 } {}
		Buffer(void* data, size_t size) : m_data{ (uint8_t*)data }, m_size{ size } {}
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
		~Buffer()
		{
			Release();
		}

		Buffer& operator=(const Buffer& other)
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

		static Buffer Copy(const void* data, size_t size)
		{
			Buffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.m_data, data, size);
			return buffer;
		}

		// size in bytes
		void Allocate(size_t size)
		{
			delete[] m_data;
			m_data = nullptr;

			if (size == 0)
				return;

			m_data = new uint8_t[size];
			m_size = size;
		}

		void Release()
		{
			delete[] m_data;
			m_data = nullptr;
			m_size = 0;
		}

		void ZeroInitialize()
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

		uint8_t* ReadBytes(size_t size, size_t offset)
		{
			KB_CORE_ASSERT(offset + size <= m_size, "Buffer overflow!");
			uint8_t* buffer = new uint8_t[size];
			memcpy(buffer, m_data + offset, size);
			return buffer;
		}

		void Write(void* data, size_t size, size_t offset = 0)
		{
			KB_CORE_ASSERT(offset + size <= m_size, "Buffer overflow!");
			memcpy(m_data + offset, data, size);
		}

		operator bool() const { return m_data; }

		uint8_t& operator[](int index) { return m_data[index]; }

		uint8_t operator[](int index) const { return m_data[index]; }

		template <typename T>
		T* As() const { return (T*)m_data; }

		inline size_t size() const { return m_size; }
		inline void* get() { return (void*)m_data; }
		inline const void* get() const { return (void*)m_data; }
	private:
		size_t m_size;
		uint8_t* m_data;
	};
}

#endif
