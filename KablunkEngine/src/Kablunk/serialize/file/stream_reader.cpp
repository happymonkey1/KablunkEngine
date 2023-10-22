#include "kablunkpch.h"

#include "Kablunk/serialize/file/stream_reader.h"

namespace kb::serialize::file
{ // start namespace kb::serialize::file

auto stream_reader::read_buffer(size_t p_size /*= 0*/) noexcept -> Buffer
{
    if (!p_size)
        read_data_unsafe((char*)(&p_size), sizeof(size_t));

    Buffer buffer{ p_size };

    read_data_unsafe(static_cast<char*>(buffer.get()), buffer.size());

    return buffer;
}

auto stream_reader::read_string() noexcept -> std::string
{
    size_t str_size;
    read_data_unsafe((char*)&str_size, sizeof(size_t));
    
    std::string str;
    str.resize(str_size);
    read_data_unsafe((char*)str.data(), sizeof(char) * str_size);

    return str;
}

} // end namespace kb::serialize::file
