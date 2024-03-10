#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/owning_buffer.h"

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace kb::serialize::file
{ // start namespace kb::serialize::file

class stream_writer
{
public:
    virtual ~stream_writer() = default;

    virtual bool is_good() const noexcept = 0;
    virtual u64 get_stream_cursor() noexcept = 0;
    virtual void set_stream_cursor(u64 p_new_cursor) noexcept = 0;
    virtual bool write_data(const char* data, size_t p_size) noexcept = 0;

    operator bool() const noexcept { return is_good(); }

    auto write_buffer(const owning_buffer& p_buffer, bool p_write_size = true) noexcept -> void;
    auto write_zero(u64 p_size) noexcept -> void;
    auto write_string(const std::string& p_str) noexcept -> void;

    template <typename T>
    auto write_trivial(const T& p_trivial) noexcept -> void
    {
        bool success = write_data((char*)&p_trivial, sizeof(T));
        KB_CORE_ASSERT(success, "[stream_writer]: failed to write trivial type!");
    }

    template <typename T>
    auto write_object(const T& p_object) noexcept -> void
    {
        T::serialize(this, p_object);
    }

    template <typename key_t, typename value_t>
    auto write_map(const std::map<key_t, value_t>& p_map, bool p_write_size = true) noexcept -> void
    {
        if (p_write_size)
            write_trivial(static_cast<size_t>(p_map.size()));

        for (const auto& [key, value] : p_map)
        {
            if constexpr (std::is_trivial<key_t>())
                write_trivial(key);
            else
                write_object(key);

            if constexpr (std::is_trivial<value_t>())
                write_trivial(value);
            else
                write_object(value);
        }
    }

    template <typename value_t>
    auto write_map(const std::map<std::string, value_t>& p_map, bool p_write_size = true) noexcept -> void
    {
        if (p_write_size)
            write_trivial(static_cast<size_t>(p_map.size()));

        for (const auto& [key, value] : p_map)
        {
            write_string(key);

            if constexpr (std::is_trivial<value_t>())
                write_trivial(value);
            else
                write_object(value);
        }
    }

    template <typename key_t, typename value_t>
    auto write_unordered_map(const kb::unordered_flat_map<key_t, value_t>& p_map, bool p_write_size = true) noexcept -> void
    {
        if (p_write_size)
            write_trivial(static_cast<size_t>(p_map.size()));

        for (const auto& [key, value] : p_map)
        {
            if constexpr (std::is_trivial<key_t>())
                write_trivial(key);
            else
                write_object(key);

            if constexpr (std::is_trivial<value_t>())
                write_trivial(value);
            else
                write_object(value);
        }
    }

    template <typename value_t>
    auto write_unordered_map(const std::map<std::string, value_t>& p_map, bool p_write_size = true) noexcept -> void
    {
        if (p_write_size)
            write_trivial(static_cast<size_t>(p_map.size()));

        for (const auto& [key, value] : p_map)
        {
            write_string(key);

            if constexpr (std::is_trivial<value_t>())
                write_trivial(value);
            else
                write_object(value);
        }
    }

    template <typename value_t>
    auto write_vec(const std::vector<value_t>& p_vec, bool p_write_size = true) noexcept -> void
    {
        if (p_write_size)
            write_trivial(static_cast<size_t>(p_vec.size()));

        for (const auto& val : p_vec)
        {
            if constexpr (std::is_trivial<value_t>())
                write_trivial(val);
            else
                write_object(val);
        }
    }

    template <>
    auto write_vec(const std::vector<std::string>& p_vec, bool p_write_size) noexcept -> void
    {
        if (p_write_size)
            write_trivial(static_cast<size_t>(p_vec.size()));

        for (const auto& val : p_vec)
            write_string(val);
    }
};

} // end namespace kb::serialize::file
