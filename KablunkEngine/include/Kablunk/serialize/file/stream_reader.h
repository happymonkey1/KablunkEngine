#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Buffer.h"

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <type_traits>

namespace kb::serialize::file
{ // start namespace kb::serialize::file

// forward declaration
class stream_reader;

namespace concepts
{ // start namespace ::concepts

// ensure that the template type is trivial
template <typename T>
concept is_trivial_type = std::is_trivial_v<T>;

// ensure that the template type is deserializable by the kablunk engine
template <typename T>
concept is_deserializable_type = requires(T t, stream_reader* p_stream_reader_ptr)
{
    T::deserialize(p_stream_reader_ptr, t);
};

} // end namespace ::concepts

class stream_reader
{
public:
    virtual ~stream_reader() = default;

    // check if the underlying stream is valid
    virtual auto is_good() const noexcept -> bool  = 0;
    // get the cursor position for the underlying stream
    virtual auto get_stream_cursor() noexcept -> u64 = 0;
    // set the cursor position for the underlying stream
    virtual auto set_stream_cursor(u64 p_new_cursor) noexcept -> void = 0;
    // (unsafe) read into a raw byte buffer
    virtual auto read_data_unsafe(char* p_destination_ptr, size_t p_size) noexcept -> bool = 0;

    // overloaded bool operator which checks if the underlying stream is valid
    operator bool() const { return is_good(); }

    // read data into the buffer reference parameter with `p_size` bytes
    auto read_buffer(size_t p_size = 0) noexcept -> Buffer;
    // read data into the string reference parameter from the underlying stream.
    // the size of the string is also retrieved from the underlying stream.
    auto read_string() noexcept -> std::string;

    // read a trivial type into the reference parameter.
    template <concepts::is_trivial_type T>
    auto read_trivial_into(T& p_trivial) noexcept -> void
    {
        bool success = read_data_unsafe((char*)&p_trivial, sizeof(T));
        KB_CORE_ASSERT(success, "[stream_reader]: failed to read into trivial type!");
    }

    // read a non-trivial type into the reference parameter using a statically defined deserialize
    // method defined on the type.
    template <concepts::is_deserializable_type T>
    auto read_object_into(T& p_dst) noexcept -> void { T::deserialize(this, p_dst); }

    // read key, value pairs into the reference map parameter
    template <typename key_t, typename value_t>
    auto read_map(std::map<key_t, value_t>& p_map, size_t p_size = 0) noexcept -> void
    {
        if (!p_size)
            read_trivial_into(p_size);

        for (size_t i = 0; i < p_size; ++i)
        {
            key_t key;
            if constexpr (std::is_trivial<key_t>())
                read_trivial_into<key_t>(key);
            else
                read_object_into<key_t>(key);

            value_t value;
            if constexpr (std::is_trivial<value_t>())
                read_trivial_into<value_t>(p_map.at(key));
            else
                read_object_into<value_t>(p_map.at(key));
        }
    }

    // read key, value pairs into the reference map parameter
    template <typename value_t>
    auto read_map(std::map<std::string, value_t>& p_map, size_t p_size = 0) noexcept -> void
    {
        if (!p_size)
            read_trivial_into(p_size);

        for (size_t i = 0; i < p_size; ++i)
        {
            key_t key = read_string();

            value_t value;
            if constexpr (std::is_trivial<value_t>())
                read_trivial_into<value_t>(p_map.at(key));
            else
                read_object_into<value_t>(p_map.at(key));
        }
    }

    // read key, value pairs into the reference unordered map parameter
    template <typename key_t, typename value_t>
    auto read_unordered_map(std::unordered_map<key_t, value_t>& p_map, size_t p_size = 0) noexcept  -> void
    {
        if (!p_size)
            read_trivial_into(p_size);

        for (size_t i = 0; i < p_size; ++i)
        {
            key_t key;
            if constexpr (std::is_trivial<key_t>())
                read_trivial_into<key_t>(key);
            else
                read_object_into<key_t>(key);

            value_t value;
            if constexpr (std::is_trivial<value_t>())
                read_trivial_into<value_t>(p_map.at(key));
            else
                read_object_into<value_t>(p_map.at(key));
        }
    }

    // read key, value pairs into the reference unordered map parameter
    template <typename value_t>
    auto read_unordered_map(std::unordered_map<std::string, value_t>& p_map, size_t p_size = 0) noexcept -> void
    {
        if (!p_size)
            read_trivial_into(p_size);

        for (size_t i = 0; i < p_size; ++i)
        {
            std::string key = read_string();

            value_t value;
            if constexpr (std::is_trivial<value_t>())
                read_trivial_into<value_t>(p_map.at(key));
            else
                read_object_into<value_t>(p_map.at(key));
        }
    }

    // read values into the reference vector parameter
    template <typename T>
    auto read_vec(std::vector<T>& p_vec, size_t p_size = 0) noexcept -> void
    {
        if (!p_size)
            read_trivial_into(p_size);

        p_vec.resize(p_size);

        for (size_t i = 0; i < p_size; ++i)
        {
            if constexpr (std::is_trivial<T>())
                read_trivial_into<T>(p_vec.at(i));
            else
                read_object_into<T>(p_vec.at(i));
        }
    }

    // read strings into the reference vector parameter
    template <>
    inline auto read_vec(std::vector<std::string>& p_vec, size_t p_size) noexcept -> void
    {
        if (!p_size)
            read_trivial_into(p_size);

        p_vec.resize(p_size);

        for (size_t i = 0; i < p_size; ++i)
            p_vec[i] = read_string();
    }
};

} // end namespace kb::serialize::file
