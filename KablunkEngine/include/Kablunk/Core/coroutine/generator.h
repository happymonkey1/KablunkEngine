#pragma once

#include <coroutine>

namespace kb
{ // start namespace kb

// reference: https://en.cppreference.com/w/cpp/language/coroutines
template<typename T>
struct generator
{
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type
    {
        T m_value;

        generator get_return_object() noexcept
        {
            return generator(handle_type::from_promise(*this));
        }

        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() {}

        template<std::convertible_to<T> From>
        std::suspend_always yield_value(From&& from) noexcept
        {
            m_value = std::forward<From>(from);
            return {};
        }

        void return_void() noexcept {}
    };


    generator(handle_type h) noexcept : m_handle(h) {}

    ~generator() noexcept { m_handle.destroy(); }

    explicit operator bool() noexcept
    {
        fill();
        return !m_handle.done();
    }

    T operator()() noexcept
    {
        fill();
        m_full = false;
        return std::move(m_handle.promise().m_value);
    }

private:
    handle_type m_handle;
    bool m_full = false;

    void fill() noexcept
    {
        if (!m_full)
        {
            m_handle();

            m_full = true;
        }
    }
};

} // end namespace kb
