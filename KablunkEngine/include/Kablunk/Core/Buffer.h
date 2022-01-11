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
		Buffer(void* data, uint32_t size) : m_data{ (uint8_t*)data }, m_size{ size } {}
		Buffer(const Buffer& other) : m_data{ nullptr }, m_size{ other.m_size }
		{
			if (other.m_data != nullptr)
			{
				Allocate(m_size);
				memcpy(m_data, other.m_data, m_size);
			}
		}

		Buffer& operator=(const Buffer& other)
		{
			if (other.m_data != nullptr)
			{
				m_size = other.m_size;
				Allocate(m_size);
				memcpy(m_data, other.m_data, m_size);
			}

			return *this;
		}

		static Buffer& Copy(const void* data, uint32_t size)
		{
			Buffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.m_data, data, size);
			return std::move(buffer);
		}

		void Allocate(uint32_t size)
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
			if (m_data)
				memset(m_data, 0, m_size);
		}

		template <typename T>
		T& Read(uint32_t offset)
		{
			return *(T*)((uint8_t*)m_data + offset);
		}

		uint8_t* ReadBytes(uint32_t size, uint32_t offset)
		{
			KB_CORE_ASSERT(offset + size <= m_size, "Buffer overflow!");
			uint8_t* buffer = new uint8_t[size];
			memcpy(buffer, (uint8_t*)m_data + offset, size);
			return buffer;
		}

		void Write(void* data, uint32_t size, uint32_t offset = 0)
		{
			KB_CORE_ASSERT(offset + size <= m_size, "Buffer overflow!");
			memcpy((uint8_t*)m_data + offset, data, size);
		}

		operator bool() const { return m_data; }

		uint8_t& operator[](int index) { return ((uint8_t*)m_data)[index]; }

		uint8_t operator[](int index) const { return ((uint8_t*)m_data)[index]; }

		template <typename T>
		T* As() const { return (T*)m_data; }

		inline uint32_t size() const { return m_size; }
		inline void* get() { return m_data; }
		inline const void* get() const { return m_data; }
	private:
		uint32_t m_size;
		uint8_t* m_data;
	};
}

#endif
