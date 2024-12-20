#pragma once

#include <cstdint>
#include <cstddef>

namespace kb
{ // start namespace kb

// TODO: allocator support
template <typename T, std::size_t N>
class inlined_vector
{
public:
    static constexpr std::size_t k_fixed_capacity = N;
    using value_t = T;
    using size_type = std::size_t;
    using iterator_t = T*;
    using const_iterator_t = const T*;

    constexpr inlined_vector() noexcept = default;

    // Copy constructor
    constexpr inlined_vector(const inlined_vector& p_other) noexcept
        : m_allocated{ p_other.m_allocated }, m_size{ p_other.m_size }
    {
        KB_CORE_ASSERT(false, "not implemented!");
    }

    // Move constructor
    constexpr inlined_vector(inlined_vector&& p_other) noexcept
        : m_data{ p_other }, m_allocated{ p_other.m_allocated }, m_size{ p_other.m_size }
    {
        if (k_fixed_capacity >= p_other.m_size)
        {
            // TODO: can we just check is_trivially_copyable_v instead?
            if constexpr (std::is_trivial_v<value_t>)
                std::memcpy(m_inlined, p_other.m_inlined, sizeof(value_t) * p_other.m_size);
            else
                for (size_type i = 0; i < p_other.m_size; ++i)
                    m_inlined[i] = std::move(p_other.m_inlined[i]);
        }
        else
            p_other.m_size = 0;

        p_other.m_allocated = 0;
        p_other.m_data = nullptr;
    }

    // Destructor
    constexpr ~inlined_vector() noexcept
    {
        clear();
    }

    constexpr auto push_back(auto&& p_value) noexcept
    {
        if (m_size >= capacity())
            reserve(capacity() * 2);

        if constexpr (std::is_trivial_v<value_t>)
            std::memcpy(begin(), &p_value, sizeof(value_t));
        else
        {
            // Placement new to construct into appropriate buffer (inlined or allocated)
            ::new (begin() + m_size) value_t{ std::forward<decltype(p_value)>(p_value) };
        }

        m_size++;
    }

    constexpr auto emplace_back(auto&&... p_args) noexcept -> void
    {
        if (m_size >= capacity())
            reserve(capacity() * 2);

        // Placement new to construct into appropriate buffer (inlined or allocated)
        ::new (begin() + m_size) value_t{ std::forward<decltype(p_args)>(p_args)... };

        m_size++;
    }

    // Check whether the container has any objects stored
    constexpr auto empty() const noexcept -> bool { return m_size == 0; }
    // Returns the current number of objects stored
    constexpr auto size() const noexcept -> size_type { return m_size; }
    // Returns the maximum number of objects that can be stored
    constexpr auto capacity() const noexcept -> size_type { return m_data ? m_allocated : k_fixed_capacity; }

    // Allocate underlying buffer of size = p_reserve_size.
    // Copy existing data over to newly allocated buffer.
    // No-op if there is enough storage in inlined memory.
    constexpr auto reserve(size_type p_reserve_size) noexcept -> void
    {
        if (capacity() > p_reserve_size)
            return;

        value_t* new_buffer = new value_t[p_reserve_size];

        // TODO: can we just check is_trivially_copyable_v instead?
        if constexpr (std::is_trivial_v<value_t>)
            std::memcpy(new_buffer, begin(), sizeof(value_t) * m_size);
        else
            for (size_type i = 0; i < m_size; ++i)
                new_buffer[i] = std::move(begin()[i]);

        delete[] m_data;
        m_data = new_buffer;
        m_allocated = p_reserve_size;
    }

    // Clear stored data and destruct, without releasing underlying memory
    constexpr auto clear() noexcept -> void
    {
        if constexpr (std::is_trivially_destructible_v<value_t>)
            return;

        // Manually invoke destructors because inlined objects should be destroyed
        for (size_type i = 0; i < m_size; ++i)
            begin()[i].~value_t();

        m_size = 0;
    }

    // Resize storage to specified size.
    // Destructs objects if requested size is less than the currently allocated size.
    // Default constructs objects if the buffer is grown.
    constexpr auto resize(size_type p_new_size) noexcept -> void
    {
        if (m_size > p_new_size && !std::is_trivially_destructible_v<value_t>)
            for (size_type i = p_new_size; i < m_size; ++i)
                begin()[i].~value_t();
        else if (p_new_size > m_size)
        {
            reserve(p_new_size);
            // Default construct objects in grown space
            for (size_type i = m_size; i < p_new_size; ++i)
                ::new (begin()[i]) value_t{};
        }

        m_size = p_new_size;
    }

    // Iterator methods
    constexpr auto begin() noexcept -> iterator_t { return m_data ? m_data : m_inlined; }
    constexpr auto end() noexcept -> iterator_t { return begin() + m_size; }
    constexpr auto begin() const noexcept -> const_iterator_t { return m_data ? m_data : m_inlined; }
    constexpr auto end() const noexcept -> const_iterator_t { return begin() + m_size; }
    constexpr auto cbegin() const noexcept -> const_iterator_t { return m_data ? m_data : m_inlined; }
    constexpr auto cend() const noexcept -> const_iterator_t { return begin() + m_size; }
private:
    // Heap allocated buffer when size outgrows the inlined capacity.
    // nullptr indicates that inlined storage is being used.
    T* m_data = nullptr;

    // Inlined storage
    union
    {
        T m_inlined[k_fixed_capacity];
    };

    // Maximum size of dynamically allocated buffer
    std::size_t m_allocated = 0;
    // Number of elements in storage buffer (inlined or allocated)
    std::size_t m_size = 0;
};

} // end namespace kb
