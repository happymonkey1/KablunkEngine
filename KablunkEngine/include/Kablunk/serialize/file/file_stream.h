#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/owning_buffer.h"

#include "Kablunk/serialize/file/stream_reader.h"
#include "Kablunk/serialize/file/stream_writer.h"

#include <filesystem>
#include <fstream>

namespace kb::serialize::file
{ // start namespace kb::serialize::file

class file_stream_reader : public stream_reader
{
public:
    file_stream_reader() = default;
    file_stream_reader(const std::filesystem::path& p_path);
    file_stream_reader(const file_stream_reader&) = delete;
    virtual ~file_stream_reader();

    // check if the underlying stream is valid
    virtual bool is_good() const noexcept override { return m_stream.good(); };
    // get the current cursor position in the underlying stream
    virtual u64 get_stream_cursor() noexcept { return m_stream.tellg(); }
    // set the current cursor position in the underlying stream
    virtual void set_stream_cursor(u64 p_new_cursor) noexcept { m_stream.seekg(p_new_cursor); }
    // read `p_size` bytes of data from the underlying stream into a raw byte buffer
    virtual bool read_data_unsafe(char* p_destination, size_t p_size) noexcept override;
private:
    // underlying input stream
    std::ifstream m_stream{};
};

class file_stream_writer : public stream_writer
{
public:
    file_stream_writer() = default;
    file_stream_writer(const std::filesystem::path& p_path);
    file_stream_writer(const file_stream_writer&) = delete;
    virtual ~file_stream_writer();

    // check if the underlying stream is valid
    virtual bool is_good() const noexcept override { return m_stream.good(); };
    // get the current cursor position in the underlying stream
    virtual u64 get_stream_cursor() noexcept override { return m_stream.tellp(); }
    // set the current cursor position in the underlying stream
    virtual void set_stream_cursor(u64 p_new_cursor) noexcept override { m_stream.seekp(p_new_cursor); }
    // write `p_size` bytes of data into the underlying stream from a raw byte buffer
    virtual bool write_data(const char* p_source, size_t p_size) noexcept override;
private:
    // underlying output stream
    std::ofstream m_stream{};
};

} // end namespace kb::serialize::file 
