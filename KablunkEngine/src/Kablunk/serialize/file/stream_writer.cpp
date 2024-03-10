#include "kablunkpch.h"

#include "Kablunk/serialize/file/stream_writer.h"

namespace kb::serialize::file
{ // start namespace kb::serialize::file

auto stream_writer::write_buffer(const owning_buffer& p_buffer, bool p_write_size /*= true*/) noexcept -> void
{
    if (p_write_size)
        write_trivial(p_buffer.size());

    write_data(static_cast<const char*>(p_buffer.get()), p_buffer.size());
}

auto stream_writer::write_zero(u64 p_size) noexcept -> void
{
    constexpr const char k_zero = 0;

    for (size_t i = 0; i < p_size; ++i)
        write_data(&k_zero, sizeof(char));
}

auto stream_writer::write_string(const std::string& p_str) noexcept -> void
{
    const size_t size = p_str.size();
    write_trivial(size);

    write_data(static_cast<const char*>(p_str.data()), sizeof(char) * size);
}

} // end namespace kb::serialize::file
