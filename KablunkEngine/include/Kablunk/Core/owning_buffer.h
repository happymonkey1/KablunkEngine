#pragma once
#ifndef KABLUNK_CORE_BUFFER_H
#define KABLUNK_CORE_BUFFER_H

#include "Kablunk/Core/Core.h"

namespace kb
{ // start namespace kb

class owning_buffer
{
public:
	constexpr owning_buffer() noexcept : m_data{ nullptr }, m_size{ 0 } {}

	constexpr owning_buffer(const void* data, const size_t size) noexcept
    : m_data{ new u8[size] }, m_size{size}
	{
        for (size_t i = 0; i < size; ++i)
            m_data[i] = static_cast<const u8*>(data)[i];
	}

	constexpr owning_buffer(const size_t size) noexcept
        : m_data{ nullptr }, m_size{ size }
    {
        Allocate(size);
    }

	constexpr owning_buffer(const owning_buffer& other) noexcept
        : m_data{ nullptr }, m_size{other.m_size}
	{
        if (!other.m_data)
            return;

        Allocate(m_size);
        if (m_data && other.m_data)
        {
            for (size_t i = 0; i < m_size; ++i)
                m_data[i] = static_cast<const u8*>(other.m_data)[i];
        }
	}

	constexpr owning_buffer(owning_buffer&& other) noexcept
		: m_data{ other.m_data }, m_size{ other.m_size }
	{
		other.m_size = 0;
		other.m_data = nullptr;
	}


	constexpr ~owning_buffer() noexcept
	{
		Release();
	}

	inline constexpr owning_buffer& operator=(const owning_buffer& other) noexcept
	{
        if (this == &other)
            return *this;

		if (other.m_data != nullptr)
		{
			m_size = other.m_size;
			Allocate(m_size);
			if (m_data && other.m_data)
			{
                for (size_t i = 0; i < m_size; ++i)
                    m_data[i] = static_cast<const u8*>(other.m_data)[i];
			}
		}

		return *this;
	}

	constexpr owning_buffer& operator=(owning_buffer&& other) noexcept
	{
        const auto tmp_size = m_size;
        m_size = other.m_size;
	    other.m_size = tmp_size;

        const auto tmp = m_data;
        m_data = other.m_data;
	    other.m_data = tmp;
		return *this;
	}

	inline static constexpr owning_buffer Copy(const void* data, size_t size)
	{
		return owning_buffer{ data, size };
	}

	// size in bytes
	inline constexpr void Allocate(const size_t size) noexcept
	{
		if (m_data)
		{
			delete[] m_data;
			m_data = nullptr;
		}

		if (size == 0)
        {
            KB_CORE_WARN("[owning_buffer]: allocating a buffer of size 0!");
			return;
        }

		//m_data = new uint8_t[size];
		m_data = new u8[size];
		m_size = size;
	}

	inline constexpr void Release() noexcept
	{
        if (!m_data)
            return;

        delete[] m_data;
		m_data = nullptr;
		m_size = 0;
	}

	inline constexpr void zero() noexcept
	{
		if (m_data && m_size > 0)
		{
            for (size_t i = 0; i < m_size; ++i)
                m_data[i] = 0;
		}
		else
			KB_CORE_ERROR("Trying to Zero Initialize an invalid buffer!");
	}

	template <typename T>
	constexpr T& Read(uint32_t offset) noexcept
	{
		KB_CORE_ASSERT(offset + sizeof(T) <= m_size, "trying to access memory out of bounds!");
		return *reinterpret_cast<T*>(m_data + offset);
	}

	inline constexpr void Write(const void* data, const size_t size, const size_t offset = 0) noexcept
	{
		KB_CORE_ASSERT(offset + size <= m_size, "owning_buffer overflow!");
        for (size_t i = 0; i < size; ++i)
            m_data[i + offset] = static_cast<const u8*>(data)[i];
	}

    inline constexpr void write(const char* p_data, const size_t p_size) noexcept
	{
        KB_CORE_ASSERT(p_size <= m_size, "owning_buffer overflow!");
        for (size_t i = 0; i < p_size; ++i)
            m_data[i] = reinterpret_cast<const u8*>(p_data)[i];
	}

    // take a pointer by reference, moving the pointer into the `owning_buffer`,
    // which will now manage the de-allocation
    constexpr auto own(void*& p_data, const size_t p_size) noexcept -> void
	{
        m_data = static_cast<u8*>(p_data);
        m_size = p_size;
	}

	constexpr operator bool() const noexcept { return m_data; }

	constexpr u8& operator[](size_t index) noexcept { return m_data[index]; }

	constexpr u8 operator[](size_t index) const noexcept { return m_data[index]; }

	template <typename T>
	T* As() const noexcept { return reinterpret_cast<T*>(m_data); }

	inline constexpr size_t size() const noexcept { return m_size; }
	inline constexpr void* get() noexcept { return (void*)m_data; }
    inline constexpr void* data() noexcept { return (void*)m_data; }
	inline constexpr const void* get() const noexcept { return static_cast<void*>(m_data); }

private:
    // pointer to the head of the block of memory
    u8* m_data;
	// capacity of the buffer
	size_t m_size;
};
} // end namespace kb

#endif
