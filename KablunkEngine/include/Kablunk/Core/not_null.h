#pragma once

#include "Kablunk/Core/KablunkAPI.h"

#include <type_traits>
#include <optional>

namespace kb
{ // start namespace kb

// wrapper for a non-nullable pointer
// usage: not_null<T>{}
template <typename T>
class not_null
{
public:
    // based on https://github.com/carbon-language/carbon-lang/blob/trunk/explorer/base/nonnull.h
    
    // alias for a non-nullable pointer ex: not_null<T*> instead of T*
    // sanitizers enforce this on assignment, return, and when passed as an argument
    template <typename T, typename std::enable_if_t<std::is_pointer_v<T>>* = nullptr>
    using not_null_t = T KB_NOT_NULL;
public:
    // default constructor
    not_null() = default;
    // copy constructor
    not_null(const not_null& p_other) noexcept : m_data{ p_other.m_data } { }
    // move constructor
    not_null(not_null&& p_other) noexcept : m_data{} { std::swap(m_data, p_other.m_data); }
    // construct a non-nullable instance from a non-nullable pointer
    not_null(not_null_t<T*> p_ptr) noexcept : m_data{ p_ptr } {}
    // no constructor for nullptr
    not_null(std::nullptr_t) = delete;
    // destructor
    ~not_null() = default;

    // try get a raw pointer, aborts when the optional pointer is empty
    inline auto get() -> not_null_t<T*>
    {
        KB_CORE_ASSERT(m_data.has_value(), "[not_null]: tried unwrapping optional pointer but it was empty!");
        return *m_data;
    }

    // try get an immutable raw pointer, aborts when the optional pointer is empty
    inline auto get() const -> not_null_t<const T*>
    {
        KB_CORE_ASSERT(m_data.has_value(), "[not_null]: tried unwrapping optional pointer but it was empty!");
        return *m_data;
    }

    // get a mutable reference to the underlying option
    inline auto get_option() noexcept -> std::optional<not_null_t<T*>>& { return m_data; }
    // get an immutable reference to the underlying option
    inline auto get_option() const noexcept -> const std::optional<not_null_t<T*>>& { return m_data; }

    // check whether there actually is a value (the option is not empty)
    inline auto is_valid() const noexcept -> bool { return m_data.has_value(); }

    // copy assign operator
    auto operator=(const not_null& p_other) noexcept -> not_null&
    {
        m_data = p_other.m_data;
        return *this;
    }

    // move assign operator
    auto operator=(not_null&& p_other) noexcept -> not_null&
    {
        std::swap(m_data, p_other.m_data);
        return *this;
    }

    // no assignment to nullptr
    auto operator=(std::nullptr_t) -> not_null& = delete;

    // assignment to a non nullable (attribute) pointer 
    auto operator=(not_null_t<T*> p_ptr) -> not_null& noexcept
    {
        m_data = p_ptr;
        return *this;
    }

    // de-reference operator
    auto operator*() noexcept -> not_null_t<T&>
    { 
        KB_CORE_ASSERT(m_data.has_value(), "[not_null]: operator*() tried to de-reference empty option!");
        return *m_data; 
    }

    // immutable de-reference operator
    auto operator*() const noexcept -> not_null_t<const T&>
    {
        KB_CORE_ASSERT(m_data.has_value(), "[not_null]: operator*() tried to de-reference empty option!");
        return *m_data;
    }

    // member access operator
    auto operator->() noexcept -> not_null_t<T*>
    { 
        KB_CORE_ASSERT(m_data.has_value(), "[not_null]: operator->() tried to de-reference empty option!");
        return m_data.value(); 
    }

    // immutable member access operator
    auto operator->() const noexcept -> not_null_t<const T*>
    {
        KB_CORE_ASSERT(m_data.has_value(), "[not_null]: operator->() tried to de-reference empty option!");
        return m_data.value();
    }

#if 0
    template <typename F>
    constexpr auto transform(F&& f)
    {
        if constexpr (m_data.has_value())
            return std::invoke(std::forward<F>(f), *m_data);
        else
            return std::nullopt;
    }
#endif

private:
    std::optional<not_null_t<T*>> m_data{};
};

} // end namespace kb
